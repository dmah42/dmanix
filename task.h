#ifndef TASK_H_
#define TASK_H_

#include <stdint.h>

namespace task {

void Initialize();
uint32_t Fork();
void Switch();
uint32_t PID();

}

#endif  // TASK_H_
