#include "timer.h"

#include "io.h"
#include "isr.h"
#include "screen.h"

namespace timer {
namespace {

uint32_t tick = 0;

void Callback(const isr::Registers& regs) {
  ++tick;
  screen::puts("Tick: ");
  screen::putd(tick);
  screen::putc('\n');
}

}  // namespace

void Initialize(uint32_t frequency) {
  isr::RegisterHandler(IRQ0, &Callback);

  // PIT runs at 1193180Hz
  const uint32_t divisor = 1193180 / frequency;

  // TODO: Check divisor fits in 0xFFFF

  io::outb(0x43, 0x36);
  io::outb(0x40, divisor & 0xFF);
  io::outb(0x40, (divisor >> 8) & 0xFF);
}

}  // namespace timer
