#include "syscall.h"

#include "base/array_size.h"
#include "base/assert.h"
#include "interrupt/isr.h"
#include "screen.h"
#include "task.h"

#define DEFN_SYSCALL0(fn, num)                      \
int fn() {                                       \
  int a;                                            \
  asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
  return a;                                         \
}

#define DEFN_SYSCALL1(fn, num, P1)                    \
int fn(P1 p1) {                                    \
  int a;                                              \
  asm volatile("int $0x80" : "=a" (a)                 \
      : "0" (num), "b" (reinterpret_cast<int>(p1)));  \
  return a;                                           \
}

#define DEFN_SYSCALL2(fn, num, P1, P2)           \
int fn(P1 p1, P2 p2) {                        \
  int a;                                         \
  asm volatile("int $0x80" : "=a" (a)            \
      : "0" (num),                               \
        "b" (reinterpret_cast<int>(p1)),         \
        "c" (reinterpret_cast<int>(p2)));        \
  return a;                                      \
}

namespace syscall {

// TODO(dominic): better way of registering these.
DEFN_SYSCALL1(screen_puts, 0, const char*)
DEFN_SYSCALL0(task_fork, 1)

namespace {

void* syscalls[] = {
  reinterpret_cast<void*>(&screen::puts),
  reinterpret_cast<void*>(&task::Fork)
};

void Handler(isr::Registers* regs) {
  ASSERT(regs->eax < ARRAY_SIZE(syscalls));

  void* location = syscalls[regs->eax];

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
        "r" (regs->ebx), "r" (location));
}

}  // namespace

void Initialize() {
  isr::RegisterHandler(0x80, Handler);
}

void Shutdown() {
  isr::UnregisterHandler(0x80, Handler);
}

}  // namespace syscall
