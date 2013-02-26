#include "heap.h"

#include <new>

#include "memory/paging.h"

#define HEAP_INDEX_SIZE     0x20000
#define HEAP_MAGIC          0x1FF
#define HEAP_MIN_SIZE       0x70000

#define PAGE_ALIGN          0x1000

namespace memory {
struct Page;

extern Directory* kernel_directory;

void AllocFrame(Page* page, bool is_kernel, bool is_writeable);
void FreeFrame(Page* page);
Page* GetPage(uint32_t address, bool make, Directory* dir);

// static
Heap* Heap::Create(uint32_t start, uint32_t end_addr, uint32_t max,
                   bool supervisor, bool readonly) {
  void* heap_mem = kalloc(sizeof(Heap));
  return new (heap_mem) Heap(start, end_addr, max, supervisor, readonly);
}

// static
void Heap::Destroy(Heap* heap) {
  heap->~Heap();
  kfree(heap);
}

void* Heap::Alloc(uint32_t size, bool page_align) {
  ASSERT(size <= 1 << 22);
  uint32_t new_size = size + sizeof(Header) + sizeof(Footer);
  uint32_t iterator = FindSmallestHole(new_size, page_align);
  if (iterator == (uint32_t) -1) {
    // Didn't find a hole
    uint32_t old_length = end_address - start_address;
    uint32_t old_end_address = end_address;

    Expand(old_length + new_size);
    uint32_t new_length = end_address - start_address;

    iterator = 0;
    int32_t idx = -1; uint32_t value = 0x0;
    while (iterator < index.get_size()) {
      uint32_t tmp = (uint32_t) index.Lookup(iterator);
      if (tmp > value) {
        value = tmp;
        idx = iterator;
      }
      ++iterator;
    }

    // If we didn't find ANY headers, we need to add one.
    if (idx == -1) {
      Header* header = (Header*) old_end_address;
      header->magic = HEAP_MAGIC;
      header->size = new_length - old_length;
      header->is_hole = 1;

      Footer* footer = (Footer*) (old_end_address + header->size - sizeof(Footer));
      footer->magic = HEAP_MAGIC;
      footer->size = header->size;

      index.Insert(header);
    } else {
      // Adjust the last header
      Header* header = index.Lookup(idx);
      header->size += new_length - old_length;

      // Rewrite the footer
      Footer* footer = (Footer*) ((uint32_t)header + header->size - sizeof(Footer));
      footer->size = header->size;
      footer->magic = HEAP_MAGIC;
    }
    // We now have enough space - recurse and try again.
    return Alloc(size, page_align);
  }
  Header* orig_header = (Header*) index.Lookup(iterator);
  uint32_t orig_pos = (uint32_t) orig_header;
  uint32_t orig_size = orig_header->size;
  // Should we split the hole?
  if (orig_size - new_size < sizeof(Header) + sizeof(Footer)) {
    // Just increase the requested size
    size += orig_size - new_size;
    new_size = orig_size;
  }

  // If we need to page align, make a new hole
  if (page_align && (orig_pos & 0xFFFFF000)) {
    uint32_t new_location = orig_pos + PAGE_ALIGN -
        (orig_pos & (PAGE_ALIGN-1)) - sizeof(Header);
    Header* hole_header = (Header*) orig_pos;
    hole_header->size = PAGE_ALIGN - (orig_pos & (PAGE_ALIGN-1)) - sizeof(Header);
    hole_header->magic = HEAP_MAGIC;
    hole_header->is_hole = 1;

    Footer* hole_footer = (Footer*) ((uint32_t)new_location - sizeof(Footer));
    hole_footer->magic = HEAP_MAGIC;
    hole_footer->size = hole_header->size;

    orig_pos = new_location;
    orig_size = orig_size - hole_header->size;
  } else {
    index.Remove(iterator);
  }

  Header* block_header = (Header*) orig_pos;
  block_header->magic = HEAP_MAGIC;
  block_header->is_hole = 0;
  block_header->size = new_size;

  Footer* block_footer = (Footer*) (orig_pos + sizeof(Header) + size);
  block_footer->magic = HEAP_MAGIC;
  block_footer->size = block_header->size;

  // Do we need a new hole after the block?
  if (orig_size - new_size > 0) {
    Header* hole_header = (Header*) (orig_pos + sizeof(Header) + size + sizeof(Footer));
    hole_header->magic = HEAP_MAGIC;
    hole_header->is_hole = 1;
    hole_header->size = orig_size - new_size;

    Footer* hole_footer = (Footer*) ((uint32_t)hole_header + orig_size - new_size - sizeof(Footer));
    if ((uint32_t)hole_footer < end_address) {
      hole_footer->magic = HEAP_MAGIC;
      hole_footer->size = hole_header->size;
    }
    index.Insert(hole_header);
  }
  return (void*)((uint32_t)block_header + sizeof(Header));
}

void Heap::Free(void* p) {
  if (p == NULL)
    return;

  Header* header = (Header*) ((uint32_t) p - sizeof(Header));
  ASSERT(header->magic == HEAP_MAGIC);

  Footer* footer = (Footer*) ((uint32_t)header + header->size - sizeof(Footer));
  ASSERT(footer->magic == HEAP_MAGIC);
  ASSERT(footer->size == header->size);

  header->is_hole = 1;

  bool do_add = true;

  // Coalesce left
  Footer* left_footer = (Footer*) ((uint32_t) header - sizeof(Footer));
  Header* left_header = (Header*) ((uint32_t) header - left_footer->size);
  if (left_footer->magic == HEAP_MAGIC && left_header->is_hole == 1) {
    uint32_t cache_size = header->size;
    header = left_header;
    header->size += cache_size;
    footer->size = header->size;
    do_add = false;
  }

  // Coalesce right
  Header* right_header = (Header*) ((uint32_t)footer + sizeof(Footer));
  if (right_header->magic == HEAP_MAGIC && right_header->is_hole) {
    header->size += right_header->size;
    Footer* right_footer = (Footer*)((uint32_t)right_header + right_header->size - sizeof(Footer));
    footer = right_footer;
    footer->size = header->size;
    uint32_t iterator = 0;
    while ((iterator < index.get_size()) && index.Lookup(iterator) != right_header)
      ++iterator;

    ASSERT(iterator < index.get_size());

    index.Remove(iterator);
  }

  // Can we contract?
  if ((uint32_t)footer + sizeof(Footer) == end_address) {
    uint32_t old_length = end_address - start_address;
    uint32_t new_length = Contract((uint32_t)header - start_address);
    if (header->size - (old_length - new_length) > 0) {
      header->size -= old_length - new_length;
      footer = (Footer*) ((uint32_t)header + header->size - sizeof(Footer));
      footer->magic = HEAP_MAGIC;
      footer->size = header->size;
    } else {
      uint32_t iterator = 0;
      while ((iterator < index.get_size()) && (index.Lookup(iterator) != right_header))
        ++iterator;
      if (iterator < index.get_size())
        index.Remove(iterator);
    }
  }

  if (do_add)
    index.Insert(header);
}

void Heap::Dump() {
  screen::puts("Dumping Heap...\n");
  VisitAllHeaders(DumpHeader);
  screen::puts("Dump complete\n");
}

void Heap::Verify() {
  screen::puts("Verifying Heap... ");
  VisitAllHeaders(VerifyHeader);
  screen::puts("Verify complete\n");
}

// static
bool Heap::HeaderLessThan(Header* const& a, Header* const& b) {
  return a->size < b->size;
}

// static
void Heap::DumpHeader(Header* header) {
  VerifyHeader(header);
  uint32_t size = header->size - (sizeof(Header) + sizeof(Footer));
  uint32_t p = (uint32_t) header + sizeof(Header);
  screen::Printf("  0x%x %c %u (0x%x)\n",
                 p, header->is_hole == 1 ? 'F' : 'A', size, size);
}
 
// static
void Heap::VerifyHeader(Header* header) {
  ASSERT(header->magic == HEAP_MAGIC);
  Footer* f = (Footer*)((uint32_t) header + header->size - sizeof(Footer));
  ASSERT(f->magic == HEAP_MAGIC);
  ASSERT(f->size == header->size);
}

Heap::Heap(uint32_t start, uint32_t end_addr, uint32_t max,
           bool supervisor, bool readonly)
    : index((void*) start, HEAP_INDEX_SIZE, HeaderLessThan),
      end_address(end_addr),
      max_address(max),
      supervisor(supervisor),
      readonly(readonly) {
  ASSERT(start % PAGE_ALIGN == 0);
  ASSERT(end_addr % PAGE_ALIGN == 0);

  // Allow for index array.
  start += sizeof(Header*) * HEAP_INDEX_SIZE;
  if ((start & 0xFFFFF000) != 0) {
    start &= 0xFFFFF000;
    start += PAGE_ALIGN;
  }
  start_address = start;

  Header* hole = (Header*) start;
  hole->size = end_addr - start;
  hole->magic = HEAP_MAGIC;
  hole->is_hole = 1;

  Footer* hole_footer = (Footer*) (((uint32_t) hole) + hole->size - sizeof(Footer));
  hole_footer->magic = HEAP_MAGIC;
  hole_footer->size = hole->size;

  index.Insert(hole);
}

Heap::~Heap() {
  // Ensure everything has been freed.
  // TODO: Reenable once task allocations are freed
  //ASSERT(index.get_size() == 1);
  //Header* hole = (Header*) index.Lookup(0);
  //ASSERT(hole->magic == HEAP_MAGIC);
  //ASSERT(hole->is_hole == 1);
  //ASSERT(hole->size == end_address - start_address);
}

int32_t Heap::FindSmallestHole(uint32_t size, bool page_align) const {
  uint32_t iterator = 0;
  while (iterator < index.get_size()) {
    Header* header = index.Lookup(iterator);
    if (page_align) {
      uint32_t location = (uint32_t) header;
      int32_t offset = 0;
      if (((location + sizeof(Header)) & 0xFFFFF000) != 0)
        offset = PAGE_ALIGN - (location + sizeof(Header)) % PAGE_ALIGN;
      int32_t hole_size = (int32_t)header->size - offset;
      if (hole_size >= (int32_t) size)
        break;
    } else if (header->size >= size)
      break;
    ++iterator;
  }

  return iterator == index.get_size() ? -1 : iterator;
}

void Heap::Expand(uint32_t new_size) {
  ASSERT(new_size > end_address - start_address);

  if ((new_size & 0xFFFFF000) != 0) {
    new_size &= 0xFFFFF000;
    new_size += PAGE_ALIGN;
  }

  ASSERT(start_address + new_size <= max_address);

  uint32_t old_size = end_address - start_address;
  uint32_t i = old_size;
  while (i < new_size) {
    memory::AllocFrame(
        memory::kernel_directory->GetPage(start_address + i, true),
        supervisor, readonly);
    i += PAGE_ALIGN;
  }
  end_address = start_address + new_size;
}

uint32_t Heap::Contract(uint32_t new_size) {
  ASSERT(new_size < end_address - start_address);

  if (new_size & PAGE_ALIGN) {
    new_size &= PAGE_ALIGN;
    new_size += PAGE_ALIGN;
  }

  if (new_size < HEAP_MIN_SIZE)
    new_size = HEAP_MIN_SIZE;

  uint32_t old_size = end_address - start_address;
  uint32_t i = old_size - PAGE_ALIGN;
  while (new_size < i) {
    memory::FreeFrame(
        memory::kernel_directory->GetPage(start_address + i, false));
    i -= PAGE_ALIGN;
  }
  end_address = start_address + new_size;
  return new_size;
}

void Heap::VisitAllHeaders(void (*callback)(Header*)) {
  uint32_t header_addr = start_address;
  while (header_addr < end_address) {
    Header* header = (Header*) header_addr;
    callback(header);
    header_addr += header->size;
  }
}

}  // namespace memory

