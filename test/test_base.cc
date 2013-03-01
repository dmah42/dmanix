#include <assert.h>
#include <stdint.h>
#include <iostream>

#include "gtest/gtest.h"

void panic(const char* msg, const char* file, uint32_t line) {
  std::cerr << msg << " at " << file << ":" << line << "\n";
  assert(false);
}
