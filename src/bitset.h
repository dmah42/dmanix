#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>

class bitset {
 public:
  explicit bitset(uint32_t n);
  ~bitset();

  void set(uint32_t address);
  void clear(uint32_t address);
  bool test(uint32_t address) const;

  uint32_t next() const;

  static const uint32_t NOT_FOUND = (uint32_t) -1;

 private:
  uint32_t* table;
  uint32_t size;
};

#endif  // BITSET_H
