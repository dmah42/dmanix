#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

namespace paging {

struct Table;
struct Directory {
//  Directory() : physicalAddress(0) {
//    memory::set((uint8_t*)tables, 0, sizeof(Table*) * ARRAY_SIZE(tables));
//    memory::set((uint8_t*)physical, 0, sizeof(uint32_t) * ARRAY_SIZE(physical));
//  }
//  ~Directory() {
//    for (uint32_t i = 0; i < 1024; ++i) {
//      tables[i]->~Table();
//      kfree(tables[i]);
//    }
//  }

  Directory* Clone();
  Table* tables[1024];
  uint32_t physical[1024];
  uint32_t physicalAddress;
};

void Initialize();
void Shutdown();

uint32_t GetPhysicalAddress(uint32_t address);

}  // namespace paging

#endif  // PAGING_H
