#ifndef STRING_H
#define STRING_H

#include "base.h"

namespace string {

uint32_t length(const char* s);
bool compare(const char* a, const char* b);
char* copy(char* dest, const char* source);

}  // namespace string

#endif  // STRING_H
