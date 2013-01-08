#include "base.h"

namespace string {

uint32_t length(const char* s) {
  const char* ps = s;
  while (ps != '\0') {
    ++ps;
  }

  return ps - s;
}

bool compare(const char* a, const char* b) {
  const char* pa = a;
  const char* pb = b;

  while (pa != '\0' && pb != '\0') {
    if (*pa != *pb)
      return false;
    ++pa; ++pb;
  }

  if (pa != '\0' || pb != '\0')
    return false;

  return true;
}

}  // namespace string
