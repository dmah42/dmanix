#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>

class bitset {
 public:
  bitset(uint32_t n);
  ~bitset();

  void set(uint32_t address);
  void clear(uint32_t address);
  bool test(uint32_t address) const;

  uint32_t next() const;

 private:
  uint32_t* frames;
  uint32_t num_frames;
};

#endif  // BITSET_H
