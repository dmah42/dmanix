#include "dt.h"
#include "paging.h"
#include "screen.h"
#include "timer.h"

void vga_test() {
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

void interrupt_test() {
  asm volatile("int $0x3");
  asm volatile("int $0x4");
}

void timer_callback(uint32_t tick) {
  screen::puts("Tick: ");
  screen::putd(tick);
  screen::putc('\n');
}

void timer_test() {
  timer::RegisterCallback(timer_callback);
}

void page_fault_test() {
  uint32_t* ptr = (uint32_t*)0xA0000000;
  uint32_t do_page_fault = *ptr;
  (void) do_page_fault;
}

int main() {//const multiboot* multiboot_ptr) {
  asm volatile("sti");
  screen::Clear();

  dt::Initialize();
  timer::Initialize(50);
  paging::Initialize();

  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts("Welcome to ");
  screen::SetColor(COLOR_BLUE, COLOR_BLACK);
  screen::puts("dma OS\n");
  screen::ResetColor();

  // vga_test();
  // interrupt_test();
  // timer_test();
  page_fault_test();

  return 0;
}
