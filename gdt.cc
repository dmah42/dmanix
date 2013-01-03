#include "base.h"

// Defined in asm.
extern "C" {
extern void gdt_flush(uint32_t);
}

namespace gdt {
namespace {

enum Segment {
  SEGMENT_NULL,
  SEGMENT_CODE,
  SEGMENT_DATA,
  SEGMENT_USER_CODE,
  SEGMENT_USER_DATA,
  NUM_SEGMENTS
};

struct entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access_flags;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

entry entries[NUM_SEGMENTS];
pointer pointer;

void set_gate(Segment segment, uint32_t base, uint32_t limit,
              uint8_t access, uint8_t granularity) {
  entries[segment].base_low = (base & 0xFFFF);
  entries[segment].base_mid = (base >> 16) & 0xFF;
  entries[segment].base_high = (base >> 24) & 0xFF;

  entries[segment].limit_low = (limit & 0xFFFF);
  entries[segment].granularity = (limit >> 16) & 0xF;
  entries[segment].granularity |= granularity;
  entries[segment].access_flags = access;
}

}  // namespace

void initialize() {
  pointer.limit = (sizeof(entry) * NUM_SEGMENTS) - 1;
  pointer.base = (uint32_t)&entries;

  set_gate(SEGMENT_NULL, 0, 0, 0, 0);
  // TODO: set the flags more explicitly
  set_gate(SEGMENT_CODE, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  set_gate(SEGMENT_DATA, 0, 0xFFFFFFFF, 0x92, 0xCF);
  set_gate(SEGMENT_USER_CODE, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  set_gate(SEGMENT_USER_DATA, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  gdt_flush((uint32_t)&pointer);
}

}
