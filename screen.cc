#include "screen.h"

#include <stdarg.h>

#include "base/assert.h"
#include "base/io.h"
#include "memory/memory.h"

#define NUM_ROWS 25
#define NUM_COLS 80

#define TAB_WIDTH 8

namespace screen {

namespace {

uint16_t* video_memory = (uint16_t*) 0xB8000;

uint8_t back_color = COLOR_BLACK;
uint8_t fore_color = COLOR_LIGHT_GREY;

struct Cursor {
  Cursor() : x(0), y(0) { }
  void Reset() { x = y = 0; }
  void Move() const {
    uint16_t index = y * NUM_COLS + x;
    io::outb(0x3D4, 14);
    io::outb(0x3D5, index >> 8);
    io::outb(0x3D4, 15);
    io::outb(0x3D5, index);
  }
  int x, y;
} cursor;

void Scroll() {
  const uint8_t default_attrib = (back_color << 4) | (fore_color & 0xF);
  const uint16_t blank = 0x20 | (default_attrib << 8);

  if (cursor.y >= 25) {
    // copy into every row from the one below
    for (int i = 0; i < NUM_ROWS - 1; ++i) {
      memory::copy(&video_memory[i * NUM_COLS],
                   &video_memory[(i+1) * NUM_COLS],
                   NUM_COLS);
    }

    // clear the last row
    memory::set(&video_memory[(NUM_ROWS - 1) * NUM_COLS], blank, NUM_COLS);

    cursor.y = NUM_ROWS - 1;
  }
}

}  // namespace

void putc(char c) {
  // backspace
  if (c == 0x08 && cursor.x > 0)
    --cursor.x;
  // tab
  else if (c == 0x09)
    cursor.x = (cursor.x + TAB_WIDTH) & ~(TAB_WIDTH - 1);
  // carriage return
  else if (c == '\r')
    cursor.x = 0;
  // newline
  else if (c == '\n') {
    cursor.x = 0;
    ++cursor.y;
  }
  // printables
  else if (c >= ' ') {
    const uint8_t attribute = (back_color << 4) | (fore_color & 0xF);
    uint16_t* video_mem_ptr = video_memory + (cursor.y * NUM_COLS + cursor.x);
    *video_mem_ptr = (attribute << 8) | c;
    ++cursor.x;
  }

  // check for wrap
  if (cursor.x >= NUM_COLS) {
    cursor.x = 0;
    ++cursor.y;
  }
  Scroll();
  cursor.Move();
}

void puts(const char* s) {
  const char* ps = s;
  while (*ps != '\0')
    putc(*ps++);
}

void puth(uint32_t hex) {
  for (int i = 28; i >= 0; i -= 4) {
    const uint8_t h = (hex >> i) & 0xf;
    if (h >= 0xA) {
      putc(h - 0xA + 'A');
    } else {
      putc(h + '0');
    }
  }
}

void putu(uint32_t dec) {
  if (dec == 0) {
    putc('0');
    return;
  }

  char c[32] = {0};
  int i = 0;
  while (dec > 0) {
    c[i++] = '0' + dec % 10;
    dec /= 10;
  }

  char c2[32] = {0};
  int j = 0;
  while (--i >= 0)
    c2[i] = c[j++];
  puts(c2);
}

void putd(int32_t dec) {
  if (dec < 0) {
    putc('-');
    dec = -dec;
  }
  putu(dec);
}

void Clear() {
  const uint8_t default_attrib = (back_color << 4) | (fore_color & 0xF);
  const uint16_t blank = 0x20 | (default_attrib << 8);

  memory::set(video_memory, blank, NUM_COLS * NUM_ROWS - 1);
  cursor.Reset();
  cursor.Move();
}

void Printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  char c;
  while ((c = *format++) != '\0') {
    if (c != '%')
      putc(c);
    else {
      c = *format++;
      switch(c) {
        case 'c':
          putc(va_arg(ap, int));
          break;

        case 'd':
          putd(va_arg(ap, int32_t));
          break;

        case 'u':
          putu(va_arg(ap, uint32_t));
          break;

        case 'x':
          puth(va_arg(ap, uint32_t));
          break;

        case 's': {
          const char* s = va_arg(ap, const char*);
          if (!s)
            s = "(null)";
          puts(s);
        } break;

        default:
          PANIC("Unsupported format");
          break;
      }
    }
  }

  va_end(ap);
}

void ResetColor() {
  SetColor(COLOR_LIGHT_GREY, COLOR_BLACK);
}

void SetColor(Color fore, Color back) {
  fore_color = fore;
  back_color = back;
}

void Mode13h() {
  uint8_t hor_regs[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x13};
  uint8_t ver_regs[] = {0x6, 0x7, 0x9, 0x10, 0x11, 0x12, 0x15, 0x16};

  uint8_t w[] = {0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x28};
  uint8_t h[] = {0xBF, 0x1F, 0x41, 0x9C, 0x8E, 0x8F, 0x96, 0xB9};

  uint8_t val = 0x63;

  io::outb(0x3C2, val);
  io::outw(0x3D4, 0x0E11);  // enable regs 0-7

  for (uint32_t a = 0; a < 7; ++a)
    io::outw(0x3D4, (uint16_t)((w[a]<<8) + hor_regs[a]));
  for (uint32_t a = 0; a < 8; ++a)
    io::outw(0x3D4, (uint16_t)((h[a]<<8) + ver_regs[a]));

  io::outw(0x3D4, 0x0008);  // vert.panning = 0

  // try chaining
  io::outw(0x3D4, 0x4014);
  io::outw(0x3D4, 0xA317);
  io::outw(0x3D4, 0x0E04);

  io::outw(0x3C4, 0x0101);
  io::outw(0x3C4, 0x0F02);  // enable writes to all planes
  io::outw(0x3CE, 0x4005);  // 256-colors
  io::outw(0x3CE, 0x0506);  // graph mode & A000-AFFF

  // Make sure we're in the write state to receive control states.
  io::inb(0x3DA);
  io::outb(0x3C0, 0x30); io::outb(0x3C0, 0x41);
  io::outb(0x3C0, 0x33); io::outb(0x3C0, 0x00);

  // EGA palette
  for (uint8_t a = 0; a < 16; ++a) {
    io::outb(0x3C0, a);
    io::outb(0x3C0, a);
  }

  // enable video
  io::outb(0x3C0, 0x20);

  // Plot pixels
  uint8_t* VGA = (uint8_t*) 0xA0000;
  for (uint16_t x = 0; x < 320; ++x) {
    for (uint16_t y = 0; y < 200; ++y) {
      uint16_t index = 320 * y + x;
      VGA[index] = y % 15 + 1;
    }
  }

}

}  // namespace screen
