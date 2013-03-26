#ifndef STRING_H
#define STRING_H

#include <stdint.h>

namespace string {

uint32_t length(const char* s);
bool compare(const char* a, const char* b);
// TODO(dominic): Reverse these so the output is last.
char* copy(char* dest, const char* source);
uint32_t find(const char* in, char delim);
void printf(char* buffer, const char* format, ...);

}  // namespace string

#endif  // STRING_H
