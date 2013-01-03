#include "timer.h"

#include "io.h"
#include "isr.h"
#include "screen.h"

namespace timer {
namespace {

uint32_t tick = 0;

void callback(const isr::registers& regs) {
  ++tick;
  screen::puts("Tick: ");
  screen::putd(tick);
  screen::putc('\n');
}

}  // namespace

void initialize(uint32_t frequency) {
  isr::register_handler(IRQ0, &callback);

  // PIT runs at 1193180Hz
  const uint16_t divisor = 1193180 / frequency;

  io::outb(0x43, 0x36);
  io::outb(0x40, divisor & 0xFF);
  io::outb(0x40, (divisor >> 8) & 0xFF);
}

}  // namespace timer
