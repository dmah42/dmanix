#ifndef PAGING_H
#define PAGING_H

#include "base.h"

namespace paging {

void Initialize();
uint32_t GetPhysicalAddress(uint32_t address);

}  // namespace paging

#endif  // PAGING_H
