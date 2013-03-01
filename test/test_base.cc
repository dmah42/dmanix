#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "gtest/gtest.h"

void panic(const char* msg, const char* file, uint32_t line) {
  fprintf(stderr, "%s at %s:%u\n", msg, file, line);
  assert(false);
}
