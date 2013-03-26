#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

namespace memory {

struct Page;
struct Table;
struct Directory {
  Directory();
  ~Directory();

  Directory* Clone();

  Page* GetPage(uint32_t address, bool make);

  Table* tables[1024];
  uint32_t physical[1024];
  uint32_t physicalAddress;
};

void Initialize(uint32_t mem_end);
void Shutdown();

uint32_t GetPhysicalAddress(uint32_t address);
void AllocateRange(uint32_t address, uint32_t size);

}  // namespace memory

#endif  // PAGING_H
