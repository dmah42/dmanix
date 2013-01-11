#include "base.h"

namespace string {

uint32_t length(const char* s) {
  const char* ps = s;
  while (*ps++ != '\0') { }

  return ps - s;
}

bool compare(const char* a, const char* b) {
  ASSERT(a != 0);
  ASSERT(b != 0)
  while (*a != '\0' && *b != '\0') {
    if (*a++ != *b++)
      return false;
  }

  return (*a == '\0' && *b == '\0');
}

char* copy(char* dest, const char* source) {
  ASSERT(dest != 0);
  ASSERT(source != 0);
  while (*source != '\0')
    *dest++ = *source++;
  return dest;
}

}  // namespace string
