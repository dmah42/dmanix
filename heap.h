#ifndef KMALLOC_H
#define KMALLOC_H

#include "base.h"
#include "ordered_list.h"

class Heap {
 public:
  static Heap* Create(uint32_t start, uint32_t end, uint32_t max,
                      bool supervisor, bool readonly);
  static void Destroy(Heap* heap);

  void* Alloc(uint32_t size, bool page_align);
  void Free(void* p);

 private:
  struct Header {
    uint32_t magic : 31;
    uint32_t is_hole : 1;
    uint32_t size;
  };

  struct Footer {
    uint32_t magic;
    Header* header;
  };

  static bool HeaderLessThan(Header* const& a, Header* const& b);

//  Heap(uint32_t start, uint32_t end, uint32_t max,
//       bool supervisor, bool readonly);
//  ~Heap();

  int32_t FindSmallestHole(uint32_t size, bool page_align) const;
  void Expand(uint32_t new_size);
  uint32_t Contract(uint32_t new_size);

  OrderedList<Header*> index;
  uint32_t start_address;
  uint32_t end_address;
  uint32_t max_address;
  bool supervisor;
  bool readonly;
};

#endif  // KMALLOC_H
