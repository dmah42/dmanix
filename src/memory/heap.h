#ifndef KMALLOC_H
#define KMALLOC_H

#include "ordered_array.h"

namespace memory {

class Heap {
 public:
  static Heap* Create(uint32_t start, uint32_t end, uint32_t max,
                      bool supervisor, bool readonly);
  static void Destroy(Heap* heap);

  void* Alloc(uint32_t size, bool page_align);
  void Free(void* p);

  void Dump();
  void Verify();

  bool Owns(void* p) const {
    return (uint32_t) p > start_address && (uint32_t) p < end_address;
  }

 private:
  struct Header {
    uint32_t magic : 9;
    uint32_t is_hole : 1;
    uint32_t size : 22;   // 4Mb max individual alloc
  };

  struct Footer {
    uint32_t magic : 9;
    uint32_t size : 22;
    uint32_t padding : 1;
  };

  static bool HeaderLessThan(Header* const& a, Header* const& b);  // NOLINT
  static void DumpHeader(Header* h);
  static void VerifyHeader(Header* h);

  Heap(uint32_t start, uint32_t end, uint32_t max,
       bool supervisor, bool readonly);
  ~Heap();

  int32_t FindSmallestHole(uint32_t size, bool page_align) const;
  void Expand(uint32_t new_size);
  uint32_t Contract(uint32_t new_size);

  void VisitAllHeaders(void (*callback)(Header* h));

  OrderedArray<Header*> index;
  uint32_t start_address;
  uint32_t end_address;
  uint32_t max_address;
  bool supervisor;
  bool readonly;
};

}  // namespace memory

#endif  // KMALLOC_H
