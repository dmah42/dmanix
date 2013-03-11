#include "process/dynamic_linker.h"

#include "base/assert.h"
#include "process/elf32_parser.h"
#include "string.h"

// From process.s
extern "C" void resolveSymbol_asm(void);

extern "C" uint32_t resolveSymbol(uint32_t library, uint32_t symbol)
{
  uint32_t a = process::current->linker()->Resolve(library, symbol);
  return a;
}

// From main.cc
extern process::Elf32Parser kelf32_parser;

namespace process {

DynamicLinker::DynamicLinker(const Elf32Parser& parser)
    : parser_(&parser) {
  global_offset_table_ =
      reinterpret_cast<uint32_t*>(parser_->GetGlobalOffsetTable());
  if (global_offset_table_ != 0) {
    ++global_offset_table_;
    *global_offset_table_ = 0;
    ++global_offset_table_;
    *global_offset_table_ = reinterpret_cast<uint32_t>(&resolve);
  }
}

uint32_t DynamicLinker::Resolve(uint32_t library, uint32_t symbol) {
  ASSERT_MSG(library == 0, "Shared libraries not yet implemented");
  // Lookup is for main program
  uint32_t* location = reinterpret_cast<uint32_t*>(
      parser_->FindDynamicSymbolAddress(symbol));
  const char* name = parser_->FindDynamicSymbolName(symbol);
  uint32_t relocate = kelf32_parser.FindSymbolAddress(name);

  // TODO(dominic): strip error_msg or have printf return it.
  char error_msg[128];
  string::printf(error_msg, "Failed to resolve symbol '%s'\n", name);

  ASSERT_MSG(relocate != 0, error_msg);
  *location = relocate;
  return relocate;
}

}  // namespace process
