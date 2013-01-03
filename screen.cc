#include "base.h"
#include "io.h"
#include "memory.h"
#include "screen.h"

#define NUM_ROWS 25
#define NUM_COLS 80

#define TAB_WIDTH 8

namespace screen {

namespace {

uint16_t* video_memory = (uint16_t*) 0xB8000;

uint8_t back_color = COLOR_BLACK;
uint8_t fore_color = COLOR_WHITE;

struct cursor {
  cursor() : x(0), y(0) { }
  void reset() { x = y = 0; }
  void move() {
    uint16_t index = y * NUM_COLS + x;
    io::outb(0x3D4, 14);
    io::outb(0x3D5, index >> 8);
    io::outb(0x3D4, 15);
    io::outb(0x3D5, index);
  }
  int x, y;
} cursor;

void scroll() {
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
  if (cursor.x >= NUM_COLS)
    puts("\r\n.. ");

  scroll();
  cursor.move();
}

void puts(const char* s) {
  const char* ps = s;
  while (*ps != '\0')
    putc(*ps++);
}

void clear() {
  const uint8_t default_attrib = (back_color << 4) | (fore_color & 0xF);
  const uint16_t blank = 0x20 | (default_attrib << 8);

  memory::set(video_memory, blank, NUM_COLS * NUM_ROWS - 1);
  cursor.reset();
  cursor.move();
}

void reset_color() {
  set_color(COLOR_WHITE, COLOR_BLACK);
}

void set_color(Color fore, Color back) {
  fore_color = fore;
  back_color = back;
}

}  // namespace screen
