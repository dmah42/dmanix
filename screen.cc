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
  for (int i = 28; i > 0; i -= 4) {
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

}  // namespace screen
