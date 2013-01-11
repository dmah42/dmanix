#ifndef NEW_H
#define NEW_H

#include "base.h"

#include <stdlib.h>

// TODO: Placement new/delete
//inline void* operator new(size_t, void* p) { return p; }
//inline void* operator new[](size_t, void* p) { return p; }
//inline void operator delete(void*, void*) { }
//inline void operator delete[](void*, void*) { }

// TODO: operator new/delete forwarding to kalloc/kfree

#endif  // NEW_H
