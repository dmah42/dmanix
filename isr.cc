#include "isr.h"

#include "base.h"
#include "io.h"
#include "screen.h"

namespace isr {

Handler handlers[256] = {0};

void RegisterHandler(Interrupt interrupt, Handler h) {
  handlers[interrupt] = h;
}

}

extern "C" {

void isr_handler(isr::Registers regs) {
  screen::puts("received interrupt:\n");
  screen::Printf(" DS:\t0x%x\n", regs.ds);
  screen::Printf(" EDI:\t0x%x ESI:\t0x%x EBP:\t0x%x ESP:\t0x%x\n",
                 regs.edi, regs.esi, regs.ebp, regs.esp);
  screen::Printf(" EBX:\t0x%x EDX:\t0x%x ECX:\t0x%x EAX:\t0x%x\n",
                 regs.ebx, regs.edx, regs.ecx, regs.eax);
  screen::Printf(" INTERRUPT: 0x%x ERR: 0x%x\n", regs.int_no, regs.err_code);
  screen::Printf(" EIP:\t0x%x CS:\t0x%x EFLAGS:\t0x%x USERESP:\t0x%x SS:\t0x%x\n",
                 regs.eip, regs.cs, regs.eflags, regs.useresp, regs.ss);

  if (isr::handlers[regs.int_no] != 0)
    isr::handlers[regs.int_no](regs);
}

void irq_handler(isr::Registers regs) {
  // Send EOI to PICS
  if (regs.int_no >= 40)
    io::outb(0xA0, 0x20);
  io::outb(0x20, 0x20);

  if (isr::handlers[regs.int_no] != 0)
    isr::handlers[regs.int_no](regs);
}

}

