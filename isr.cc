#include "base.h"
#include "screen.h"

namespace {

struct registers {
  uint32_t ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
};

}  // namespace

extern "C" {
  // TODO: handle interrupts
void isr_handler(registers regs) {
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
}
}
