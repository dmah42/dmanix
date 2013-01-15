#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

namespace paging {

void Initialize();
void Shutdown();

uint32_t GetPhysicalAddress(uint32_t address);

}  // namespace paging

#endif  // PAGING_H
