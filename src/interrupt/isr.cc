#include "isr.h"

#include <stdlib.h>

#include "base/assert.h"
#include "base/io.h"
#include "screen.h"

namespace isr {

Handler handlers[256] = {NULL};

void RegisterHandler(Interrupt interrupt, Handler h) {
  handlers[interrupt] = h;
}

void UnregisterHandler(Interrupt interrupt, Handler h) {
  ASSERT(handlers[interrupt] == h);
  handlers[interrupt] = NULL;
}

}  // namespace isr

extern "C" {
void isr_handler(isr::Registers regs) {
  if (isr::handlers[regs.int_no] != 0) {
    // TODO(dominic): Set current task register pointer to regs
    isr::handlers[regs.int_no](&regs);
    // TODO(dominic): Set current task register pointer back to oldregs
  } else {
    screen::SetColor(COLOR_BLACK, COLOR_DARK_GREEN);
    screen::puts("UNHANDLED INTERRUPT: ");
    screen::SetColor(COLOR_WHITE, COLOR_BLACK);
    screen::Printf("\n DS:\t0x%x\n", regs.ds);
    screen::Printf(" EDI:\t0x%x ESI:\t0x%x EBP:\t0x%x ESP:\t0x%x\n",
                   regs.edi, regs.esi, regs.ebp, regs.esp);
    screen::Printf(" EBX:\t0x%x EDX:\t0x%x ECX:\t0x%x EAX:\t0x%x\n",
                   regs.ebx, regs.edx, regs.ecx, regs.eax);
    screen::Printf(" INTERRUPT: 0x%x ERR: 0x%x\n", regs.int_no, regs.err_code);
    screen::Printf(" EIP:\t0x%x CS:\t0x%x EFLAGS:\t0x%x USERESP:\t0x%x "
                   "SS:\t0x%x\n",
                   regs.eip, regs.cs, regs.eflags, regs.useresp, regs.ss);
    screen::ResetColor();
    for (;;) {}
  }
}

void irq_handler(isr::Registers regs) {
  // Send EOI to PICS
  if (regs.int_no >= 40)
    io::outb(0xA0, 0x20);
  io::outb(0x20, 0x20);

  if (isr::handlers[regs.int_no] != NULL) {
    // TODO(dominic): Set current task register pointer to regs
    isr::handlers[regs.int_no](&regs);
    // TODO(dominic): Set current task register pointer back to oldregs
  }
}
}

