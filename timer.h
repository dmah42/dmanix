#ifndef TIMER_H
#define TIMER_H

#include "base.h"

namespace timer {

void Initialize(uint32_t frequency);

typedef void (*Callback)(uint32_t tick);
void RegisterCallback(Callback callback);

}

#endif  // TIMER_H
