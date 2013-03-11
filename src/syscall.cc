#include "syscall.h"

#include "base/array_size.h"
#include "base/assert.h"
#include "interrupt/isr.h"
#include "screen.h"
#include "string.h"
#include "vector.h"

#include <stdarg.h>

#define MAX_SYSCALLS 64

namespace syscall {
namespace {

int syscall0(uint32_t index) {
  int a;
  asm volatile("int $0x80" : "=a" (a) : "0" (index));
  return a;
}

int syscall1(uint32_t index, int p0) {
  int a;
  asm volatile("int $0x80" : "=a" (a) : "0" (index), "b" (p0));
  return a;
}

int syscall2(uint32_t index, int p0, int p1) {
  int a;
  asm volatile("int $0x80" : "=a" (a) : "0" (index), "b" (p0), "c" (p1));
  return a;
}

// TODO(dominic): Hashmap instead
vector<const char*, MAX_SYSCALLS> syscall_fn_lookup;
vector<void*, MAX_SYSCALLS> syscall_fns;

void Handler(isr::Registers* regs) {
  void* function = syscall_fns.at(regs->eax);

  // Push all the parameters in the correct order.
  asm volatile (" \
      push %1;    \
      push %2;    \
      push %3;    \
      push %4;    \
      push %5;    \
      call *%6;   \
      pop %%ebx;  \
      pop %%ebx;  \
      pop %%ebx;  \
      pop %%ebx;  \
      pop %%ebx"
      : "=a" (regs->eax)
      : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx),
        "r" (regs->ebx), "r" (function));
}

}  // namespace

void Initialize() {
  isr::RegisterHandler(0x80, Handler);
}

void Shutdown() {
  isr::UnregisterHandler(0x80, Handler);
}

void Register(const char* name, void* fn) {
  screen::Printf("Registering syscall '%s' at %u\n", name, syscall_fns.size());
  syscall_fn_lookup.push_back(name);
  syscall_fns.push_back(fn);
  ASSERT(syscall_fns.size() == syscall_fn_lookup.size());
}

int Call(const char* name, uint32_t num_args, ...) {
  // TODO(dominic): track num args on registration.
  va_list args;
  va_start(args, num_args);

  int result = 0;

  uint32_t index;
  for (index = 0; index < syscall_fn_lookup.size(); ++index) {
    if (string::compare(syscall_fn_lookup.at(index), name)) {
      switch (num_args) {
        case 0:
          result = syscall0(index);
          break;

        case 1:
          result = syscall1(index, va_arg(args, int));
          break;

        case 2:
          result = syscall2(index, va_arg(args, int), va_arg(args, int));
          break;

        default:
          PANIC("Bad number of arguments for syscall.");
          break;
      }
      break;
    }
  }

  // TODO(dominic): Must set errno or something here instead of trying to ASSERT
  // from within interrupt handler.
  ASSERT_MSG(index != syscall_fn_lookup.size(), "Failed to find syscall");

  va_end(args);
  return result;
}

}  // namespace syscall
