#include "string.h"

#include <stdint.h>
#include <stdlib.h>

#include "base/assert.h"

namespace string {

uint32_t length(const char* s) {
  ASSERT(s != NULL);
  const char* ps = s;
  while (*ps++ != '\0') { }
  return ps - s - 1;
}

bool compare(const char* a, const char* b) {
  ASSERT(a != NULL);
  ASSERT(b != NULL)
  while (*a != '\0' && *b != '\0') {
    if (*a++ != *b++)
      return false;
  }
  return (*a == '\0' && *b == '\0');
}

char* copy(char* dest, const char* source) {
  char* ret = dest;
  ASSERT(dest != NULL);
  ASSERT(source != NULL);
  while (*source != '\0')
    *dest++ = *source++;
  *dest = '\0';
  return ret;
}

uint32_t find(const char* s, char delim) {
  ASSERT(s != NULL);
  const char* ps = s;
  while (*ps != '\0' && *ps++ != delim) { }
  return *ps == '\0' ? static_cast<uint32_t>(-1) : ps - s - 1;
}

}  // namespace string
