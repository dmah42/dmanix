#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

namespace syscall {

void Initialize();
void Shutdown();

// TODO(dominic): Consider template to allow typename Function. This would bring
// the vector into the header though.
void Register(const char* name, void* fn);

int Call(const char* name, uint32_t num_args, ...);

}  // namespace syscall

#endif  // SYSCALL_H
