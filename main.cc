#include "dt.h"
#include "heap.h"
#include "paging.h"
#include "screen.h"
#include "timer.h"

namespace test {
void vga() {
  screen::puts("0         1         2         3         4         5         6         7       \n");
  screen::puts("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
  screen::puts("2\n");
  screen::puts("3\n");
  screen::puts("4\n");
  screen::puts("5\n");
  screen::puts("6\n");
  screen::puts("7\n");
  screen::puts("8\n");
  screen::puts("9\n");
  screen::puts("10\n");
  screen::puts("11\n");
  screen::puts("12\n");
  screen::puts("13\n");
  screen::puts("14\n");
  screen::puts("15\n");
  screen::puts("16\n");
  screen::puts("17\n");
  screen::puts("18\n");
  screen::puts("19\n");
  screen::puts("20\n");
  screen::puts("21\n");
  screen::puts("22\n");
  screen::puts("23\n");
  screen::puts("24\n");
  screen::puts("25");
}

void colors() {
  screen::puts("foreground:\n");
  screen::SetColor(COLOR_BLACK, COLOR_BLACK);
  screen::puts("black\t");
  screen::SetColor(COLOR_DARK_RED, COLOR_BLACK);
  screen::puts("dark red\t");
  screen::SetColor(COLOR_DARK_GREEN, COLOR_BLACK);
  screen::puts("dark green\t");
  screen::SetColor(COLOR_DARK_YELLOW, COLOR_BLACK);
  screen::puts("dark yellow\n");
  screen::SetColor(COLOR_DARK_BLUE, COLOR_BLACK);
  screen::puts("dark blue\t");
  screen::SetColor(COLOR_DARK_MAGENTA, COLOR_BLACK);
  screen::puts("dark magenta\t");
  screen::SetColor(COLOR_DARK_CYAN, COLOR_BLACK);
  screen::puts("dark cyan\t");
  screen::SetColor(COLOR_LIGHT_GREY, COLOR_BLACK);
  screen::puts("light grey\n");
  screen::SetColor(COLOR_GREY, COLOR_BLACK);
  screen::puts("grey\t");
  screen::SetColor(COLOR_RED, COLOR_BLACK);
  screen::puts("red\t");
  screen::SetColor(COLOR_GREEN, COLOR_BLACK);
  screen::puts("green\t");
  screen::SetColor(COLOR_YELLOW, COLOR_BLACK);
  screen::puts("yellow\n");
  screen::SetColor(COLOR_BLUE, COLOR_BLACK);
  screen::puts("blue\t");
  screen::SetColor(COLOR_MAGENTA, COLOR_BLACK);
  screen::puts("magenta\t");
  screen::SetColor(COLOR_CYAN, COLOR_BLACK);
  screen::puts("cyan\t");
  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts("white\n");

  screen::puts("background:\n");
  screen::SetColor(COLOR_BLACK, COLOR_BLACK);
  screen::puts("black\t");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_RED);
  screen::puts("dark red\t");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_GREEN);
  screen::puts("dark green\t");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_YELLOW);
  screen::puts("dark yellow\n");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_BLUE);
  screen::puts("dark blue\t");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_MAGENTA);
  screen::puts("dark magenta\t");
  screen::SetColor(COLOR_BLACK, COLOR_DARK_CYAN);
  screen::puts("dark cyan\t");
  screen::SetColor(COLOR_BLACK, COLOR_LIGHT_GREY);
  screen::puts("light grey\n");
  screen::SetColor(COLOR_BLACK, COLOR_GREY);
  screen::puts("grey\t");
  screen::SetColor(COLOR_BLACK, COLOR_RED);
  screen::puts("red\t");
  screen::SetColor(COLOR_BLACK, COLOR_GREEN);
  screen::puts("green\t");
  screen::SetColor(COLOR_BLACK, COLOR_YELLOW);
  screen::puts("yellow\n");
  screen::SetColor(COLOR_BLACK, COLOR_BLUE);
  screen::puts("blue\t");
  screen::SetColor(COLOR_BLACK, COLOR_MAGENTA);
  screen::puts("magenta\t");
  screen::SetColor(COLOR_BLACK, COLOR_CYAN);
  screen::puts("cyan\t");
  screen::SetColor(COLOR_BLACK, COLOR_WHITE);
  screen::puts("white\n");
}

void interrupt() {
  asm volatile("int $0x3");
  asm volatile("int $0x4");
}

void timer_callback(uint32_t tick) {
  screen::Printf("Tick: %d\n", tick);
}

void timer() {
  timer::RegisterCallback(timer_callback);
}

void page_fault() {
  uint32_t* ptr = (uint32_t*)0xA0000000;
  uint32_t do_page_fault = *ptr;
  (void) do_page_fault;
}

void memory() {
  screen::puts("memory test start\n");
  void* a = kalloc(8);
  void* b = kalloc(8);
  screen::Printf("a: %x, b: %x\n", a, b);

  screen::puts("about to free\n");
  kfree(b);
  kfree(a);
  screen::puts("freed\n");
  void* d = kalloc(12);
  screen::Printf("d: %x\n", d);
  kfree(d); 
}

void screen() {
  screen::puts("hex printing: ");
  screen::puth(0xDEADC0DE);
  screen::putc('\n');

  screen::puts("int printing: ");
  screen::putd(-42);
  screen::putc('\n');

  screen::puts("uint printing: ");
  screen::putd(42);
  screen::putc('\n');

  screen::Printf("Printf: %x %d %u %s\n", 0xDEADC0DE, -42, 42, "done!");
  for(;;);
}

}  // namespace test

int main() {
  screen::Clear();

  dt::Initialize();
  //test::memory();

  paging::Initialize();

  asm volatile("sti");
  timer::Initialize(50);
  
  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts("Welcome to ");
  screen::SetColor(COLOR_BLUE, COLOR_BLACK);
  screen::puts("DMA");
  screen::SetColor(COLOR_GREEN, COLOR_BLACK);
  screen::puts("NIX\n");
  screen::ResetColor();

  // test::vga();
  // test::colors();
  // test::interrupt();
  // test::timer();
  // test::page_fault();
  // test::memory();
  test::screen();

  return 0;
}
