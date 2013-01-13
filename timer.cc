#include "timer.h"

#include "io.h"
#include "isr.h"

namespace timer {
namespace {

uint32_t tick = 0;
Callback callback = 0;

void OnTick(const isr::Registers&) {
  ++tick;
  if (callback != 0)
    callback(tick);
}

}  // namespace

void Initialize(uint32_t frequency) {
  isr::RegisterHandler(IRQ0, &OnTick);

  // PIT runs at 1193180Hz
  const uint32_t divisor = 1193180 / frequency;

  // TODO: Check divisor fits in 0xFFFF

  io::outb(0x43, 0x36);
  io::outb(0x40, divisor & 0xFF);
  io::outb(0x40, (divisor >> 8) & 0xFF);
}

void RegisterCallback(Callback cb) {
  callback = cb;
}

}  // namespace timer
