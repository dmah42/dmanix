#ifndef TASK_H
#define TASK_H

#include <stdint.h>

namespace process {

void Initialize();
uint32_t Fork();
void Switch();
void ExecVE(const char* program, const char** argv, const char** env);
uint32_t PID();

void UserMode();
void KernelMode();

}  // namespace process

#endif  // TASK_H
