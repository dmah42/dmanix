#include <stdint.h>

#include "gtest/gtest.h"

void panic(const char* msg, const char* file, uint32_t line) {
  ADD_FAILURE_AT(file, line);
}
