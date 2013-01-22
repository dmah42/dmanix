#include <stdint.h>
#include <stdlib.h>

void* kalloc(uint32_t n) {
  return malloc(n);
}

void kfree(void* p) {
  free(p);
}

