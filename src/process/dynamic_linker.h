#ifndef PROCESS_DYNAMIC_LINKER_H_
#define PROCESS_DYNAMIC_LINKER_H_

#include <stdint.h>

namespace process {

class Elf32Parser;

class DynamicLinker {
 public:
  explicit DynamicLinker(const Elf32Parser& parser);
  uint32_t Resolve(uint32_t library, uint32_t symbol);

 private:
  const Elf32Parser* parser_;
  uint32_t* global_offset_table_;
};

}  // namespace process

#endif  // PROCESS_DYNAMIC_LINKER_H_
