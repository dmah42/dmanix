#include "string.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "base/assert.h"

namespace string {
namespace {

void putc(char* buffer, char c) {
  *buffer++ = c;
}

void puts(char* buffer, const char* s) {
  const char* ps = s;
  while (*ps != '\0')
    putc(buffer, *ps++);
}

void puth(char* buffer, uint32_t hex) {
  for (int i = 28; i >= 0; i -= 4) {
    const uint8_t h = (hex >> i) & 0xf;
    if (h >= 0xA) {
      putc(buffer, h - 0xA + 'A');
    } else {
      putc(buffer, h + '0');
    }
  }
}

void putu(char* buffer, uint32_t dec) {
  if (dec == 0) {
    putc(buffer, '0');
    return;
  }

  char c[32] = {0};
  int i = 0;
  while (dec > 0) {
    c[i++] = '0' + dec % 10;
    dec /= 10;
  }

  char c2[32] = {0};
  int j = 0;
  while (--i >= 0)
    c2[i] = c[j++];
  puts(buffer, c2);
}

void putd(char* buffer, int32_t dec) {
  if (dec < 0) {
    putc(buffer, '-');
    dec = -dec;
  }
  putu(buffer, dec);
}

}  // namespace

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

void printf(char* buffer, const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  char c;
  while ((c = *format++) != '\0') {
    if (c != '%') {
      putc(buffer, c);
    } else {
      c = *format++;
      switch (c) {
        case 'c':
          putc(buffer, va_arg(ap, int));
          break;

        case 'd':
          putd(buffer, va_arg(ap, int32_t));
          break;

        case 'u':
          putu(buffer, va_arg(ap, uint32_t));
          break;

        case 'x':
          puth(buffer, va_arg(ap, uint32_t));
          break;

        case 's': {
          const char* s = va_arg(ap, const char*);
          if (!s)
            s = "(null)";
          puts(buffer, s);
        } break;

        default:
          PANIC("Unsupported format");
          break;
      }
    }
  }

  va_end(ap);

  putc(buffer, '\0');
}


}  // namespace string
