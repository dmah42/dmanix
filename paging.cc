#include "paging.h"

#include "base.h"
#include "heap.h"
#include "isr.h"
#include "memory.h"
#include "screen.h"

extern uint32_t base_address;

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

Heap* kheap = 0;

namespace paging {
namespace {

struct Page {
  uint32_t present  : 1;
  uint32_t rw       : 1;
  uint32_t user     : 1;
  uint32_t accessed : 1;
  uint32_t dirty    : 1;
  uint32_t unused   : 7;
  uint32_t frame    : 20;
};

struct Table {
  Page pages[1024];
};

struct Directory {
  Table* tables[1024];
  uint32_t physical[1024];
  uint32_t physicalAddress;
};

// TODO: Remove assumption of 16Mb
const uint32_t mem_end = 0x1000000;

Directory* kernel_directory = 0;
Directory* current_directory = 0;

// bitset of used/free frames
// TODO: class
uint32_t* frames;
uint32_t num_frames;

#define INDEX_FROM_BIT(a)   (a/(8*4))
#define BIT_FROM_INDEX(a)   (a*4*8)
#define OFFSET_FROM_BIT(a)  (a%(8*4))

#define FRAME_TO_ADDR(f)    (f*0x1000)
#define ADDR_TO_FRAME(a)    (a/0x1000)

void SetFrame(uint32_t addr) {
  uint32_t frame = ADDR_TO_FRAME(addr);
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  frames[index] |= (0x1 << offset);
}

void ClearFrame(uint32_t addr) {
  uint32_t frame = ADDR_TO_FRAME(addr);
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  frames[index] &= ~(0x1 << offset);
}

bool TestFrame(uint32_t addr) {
  uint32_t frame = ADDR_TO_FRAME(addr);
  uint32_t index = INDEX_FROM_BIT(frame);
  uint32_t offset = OFFSET_FROM_BIT(frame);
  return (frames[index] & (0x1 << offset)) != 0;
}

uint32_t FindFrame() {
  for (uint32_t i = 0; i < INDEX_FROM_BIT(num_frames); ++i) {
    if (frames[i] == 0xFFFFFFFF)
      continue;
    for (uint32_t j = 0; j < 32; ++j) {
      uint32_t to_test = 0x1 << j;
      if ((frames[i] & to_test) == 0)
        return BIT_FROM_INDEX(i) + j;
    }
  }
  return (uint32_t) -1;
}

void AllocFrame(Page* page, bool is_kernel, bool is_writeable) {
  // Check if already allocated
  if (page->frame != 0)
    return;

  uint32_t index = FindFrame();
  if (index == (uint32_t) -1) {
    PANIC("No free frames.");
  }
  SetFrame(FRAME_TO_ADDR(index));
  page->present = 1;
  page->rw = is_writeable ? 1 : 0;
  page->user = is_kernel ? 0 : 1;
  page->frame = index;
}

void FreeFrame(Page* page) {
  uint32_t frame = page->frame;
  // Check if it's already free
  if (frame == 0)
    return;

  ClearFrame(frame);
  page->frame = 0;
}

void SwitchPageDirectory(Directory* dir) {
  current_directory = dir;
  asm volatile("mov %0, %%cr3" : : "r" (&dir->physical));
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

Page* GetPage(uint32_t address, bool make, Directory* dir) {
  address = ADDR_TO_FRAME(address);
  uint32_t table_index = address / 1024;

  if (dir->tables[table_index] == 0 && make) {
    uint32_t tmp;
    dir->tables[table_index] = (Table*) kalloc_pa(sizeof(Table), &tmp);
    memory::set((uint8_t*) dir->tables[table_index], (uint8_t) 0, sizeof(Table));
    dir->physical[table_index] = tmp | 0x7;  // PRESENT | RW | USER
  }

  if (dir->tables[table_index] != 0)
    return &dir->tables[table_index]->pages[address % 1024];

  return 0;
}

void PageFault(const isr::Registers& regs) {
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  bool present = !(regs.err_code & 0x1);
  bool rw = regs.err_code & 0x2;
  bool user = regs.err_code & 0x4;
  bool reserved = regs.err_code & 0x8;
  bool instruction = regs.err_code & 0x10;

  // Output error message to screen
  screen::SetColor(COLOR_BLACK, COLOR_DARK_RED);
  screen::puts("PAGE FAULT");
  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts(" (");
  if (present) screen::puts("present ");
  if (rw) screen::puts("read-only ");
  if (user) screen::puts("user-mode ");
  if (reserved) screen::puts("reserved ");
  if (instruction) screen::puts("instruction ");
  screen::puts(") at ");
  screen::puth(faulting_address);
  screen::putc('\n');
  PANIC("Page fault"); 
}

}  // namespace

void Initialize() {
  num_frames = mem_end / 0x1000;
  frames = (uint32_t*) kalloc(INDEX_FROM_BIT(num_frames));
  memory::set((uint8_t*) frames, (uint8_t) 0, INDEX_FROM_BIT(num_frames));

  // Create the page directory
  kernel_directory = (Directory*) kalloc_pa(sizeof(Directory));
  memory::set((uint8_t*) kernel_directory, (uint8_t) 0, sizeof(Directory));
  current_directory = kernel_directory;

  // Map pages in the kernel heap area.
  // Call GetPage but not AllocFrame. Tables can be created where necessary and
  // we can't allocate frames as they need to be identity mapped below.
  uint32_t i = 0;
  for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
    GetPage(i, true, kernel_directory);

  i = 0;
  while (i < base_address + 0x1000) {
    AllocFrame(GetPage(i, true, kernel_directory), false, false);
    i += 0x1000;
  }

  // Allocate the pages we mapped
  for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
    AllocFrame(GetPage(i, true, kernel_directory), false, false);

  // TODO: interrupt constants
  isr::RegisterHandler(14, PageFault);

  SwitchPageDirectory(kernel_directory);

  // Create the kernel heap
  kheap = Heap::Create(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE,
                       0xCFFFF000, false, false);
}

uint32_t GetPhysicalAddress(uint32_t address) {
  Page* page = GetPage(address, false, kernel_directory);
  return page->frame * 0x1000 + (address & 0xFFF);
}

}  // namespace paging
