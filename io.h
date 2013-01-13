#ifndef IO_H
#define IO_H

#include "base.h"

namespace io {

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

}  // namespace io

#endif  // IO_H
