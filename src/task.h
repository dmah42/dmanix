#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include <stdint.h>

namespace task {

void Initialize();
uint32_t Fork();
void Switch();
uint32_t PID();

void UserMode();

}

#endif  // TASK_H
