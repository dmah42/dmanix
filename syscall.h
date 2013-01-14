#ifndef SYSCALL_H
#define SYSCALL_H

#define DECL_SYSCALL0(fn)             int _##fn();
#define DECL_SYSCALL1(fn,p1)          int _##fn(p1);
#define DECL_SYSCALL2(fn,p1,p2)       int _##fn(p1,p2);

namespace syscall {

void Initialize();

DECL_SYSCALL1(puts, const char*)

}  // namespace syscall

#endif  // SYSCALL_H
