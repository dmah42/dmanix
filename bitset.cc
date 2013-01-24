#include "bitset.h"

#include "memory/kalloc.h"
#include "memory/memory.h"

#define INDEX_FROM_BIT(a)   (a/(8*4))
#define BIT_FROM_INDEX(a)   (a*4*8)
#define OFFSET_FROM_BIT(a)  (a%(8*4))

bitset::bitset(uint32_t num_frames) : num_frames(num_frames) {
  frames = (uint32_t*) kalloc(INDEX_FROM_BIT(num_frames));
  memory::set8((uint8_t*) frames, 0, INDEX_FROM_BIT(num_frames));
}

bitset::~bitset() {
  kfree(frames);
}

void bitset::set(uint32_t frame) {
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  frames[index] |= (0x1 << offset);
}

void bitset::clear(uint32_t frame) {
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  frames[index] &= ~(0x1 << offset);
}

bool bitset::test(uint32_t frame) const {
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  return (frames[index] & (0x1 << offset)) != 0;
}

uint32_t bitset::next() const {
  for (uint32_t i = 0; i < INDEX_FROM_BIT(num_frames); ++i) {
    if (frames[i] == (uint32_t) -1)
      continue;
    for (uint32_t j = 0; j < 32; ++j) {
      uint32_t to_test = 0x1 << j;
      if ((frames[i] & to_test) == 0)
        return BIT_FROM_INDEX(i) + j;
    }
  }
  return (uint32_t) -1;
}

