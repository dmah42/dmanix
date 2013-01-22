#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

namespace memory {

struct Table;
struct Directory {
  Directory();
  ~Directory();

  Directory* Clone();

  Table* tables[1024];
  uint32_t physical[1024];
  uint32_t physicalAddress;
};

void Initialize();
void Shutdown();

uint32_t GetPhysicalAddress(uint32_t address);

}  // namespace memory

#endif  // PAGING_H
