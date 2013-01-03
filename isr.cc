#include "isr.h"

#include "base.h"
#include "io.h"
#include "screen.h"

namespace isr {

handler handlers[256] = {0};

void register_handler(uint8_t interrupt, handler h) {
  handlers[interrupt] = h;
}

}

extern "C" {

void isr_handler(isr::registers regs) {
  screen::puts("received interrupt:\n");
  screen::puts(" DS:\t");
  screen::puth(regs.ds);
  screen::putc('\n');

  screen::puts(" EDI: ");
  screen::puth(regs.edi);
  screen::puts(" ESI: ");
  screen::puth(regs.esi);
  screen::puts(" EBP: ");
  screen::puth(regs.ebp);
  screen::puts(" ESP: ");
  screen::puth(regs.esp);
  screen::putc('\n');

  screen::puts(" EBX: ");
  screen::puth(regs.ebx);
  screen::puts(" EDX: ");
  screen::puth(regs.edx);
  screen::puts(" ECX: ");
  screen::puth(regs.ecx);
  screen::puts(" EAX: ");
  screen::puth(regs.eax);
  screen::putc('\n');

  screen::puts(" INTERRUPT: ");
  screen::puth(regs.int_no);
  screen::puts(" ERR: ");
  screen::puth(regs.err_code);
  screen::putc('\n');

  screen::puts(" EIP: ");
  screen::puth(regs.eip);
  screen::puts(" CS: ");
  screen::puth(regs.cs);
  screen::puts(" EFLAGS: ");
  screen::puth(regs.eflags);
  screen::puts(" USER_ESP: ");
  screen::puth(regs.useresp);
  screen::puts(" SS: ");
  screen::puth(regs.ss);
  screen::putc('\n');

  if (isr::handlers[regs.int_no] != 0)
    isr::handlers[regs.int_no](regs);
}

void irq_handler(isr::registers regs) {
  // Send EOI to PICS
  if (regs.int_no >= 40)
    io::outb(0xA0, 0x20);
  io::outb(0x20, 0x20);

  if (isr::handlers[regs.int_no] != 0)
    isr::handlers[regs.int_no](regs);
}

}

