#include "screen.h"

//struct multiboot {
//
//};

int main() {//const multiboot* multiboot_ptr) {
  screen::clear();
  screen::puts("Welcome to ");
  screen::set_color(COLOR_GREEN, COLOR_BLACK);
  screen::puts("DMAOS...\n");
  screen::reset_color();

  return 0xDEADC0DE;
}
