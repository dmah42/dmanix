#ifndef SYSCALL_H
#define SYSCALL_H

#define DECL_SYSCALL0(fn)             int fn();
#define DECL_SYSCALL1(fn, p1)         int fn(p1);
#define DECL_SYSCALL2(fn, p1, p2)     int fn(p1, p2);

namespace syscall {

void Initialize();
void Shutdown();

// TODO(dominic): better way of declaring these
DECL_SYSCALL1(screen_puts, const char*)
DECL_SYSCALL0(task_fork)

}  // namespace syscall

#endif  // SYSCALL_H
