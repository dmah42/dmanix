#ifndef ISR_H
#define ISR_H

#include "base.h"

const uint8_t IRQ0 = 32;
const uint8_t IRQ1 = 33;
const uint8_t IRQ2 = 34;
const uint8_t IRQ3 = 35;
const uint8_t IRQ4 = 36;
const uint8_t IRQ5 = 37;
const uint8_t IRQ6 = 38;
const uint8_t IRQ7 = 39;
const uint8_t IRQ8 = 40;
const uint8_t IRQ9 = 41;
const uint8_t IRQ10 = 42;
const uint8_t IRQ11 = 43;
const uint8_t IRQ12 = 44;
const uint8_t IRQ13 = 45;
const uint8_t IRQ14 = 46;
const uint8_t IRQ15 = 47;

namespace isr {

struct registers {
  uint32_t ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*handler)(const registers&);
void register_handler(uint8_t interrupt, handler h);

}

#endif  // ISR_H
