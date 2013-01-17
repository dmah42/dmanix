#include "paging.h"

#include <new>

#include "base/array_size.h"
#include "interrupt/isr.h"
#include "memory/heap.h"
#include "memory/memory.h"
#include "screen.h"

// from kalloc.cc
extern uint32_t base_address;

// from process.s
extern "C" void copy_page_physical(uint32_t dest, uint32_t src);

#define KHEAP_START         0xC0000000
// 4Mb initial heap
#define KHEAP_INITIAL_SIZE  0x00400000
#define KHEAP_END           (KHEAP_START + KHEAP_INITIAL_SIZE)
#define KHEAP_MAX           0xCFFFF000

memory::Heap* kheap = NULL;

namespace paging {

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
  Table() {
    memory::set((uint8_t*)pages, 0, sizeof(Page) * ARRAY_SIZE(pages));
  }
  // TODO: copy ctor
  Table* Clone(uint32_t* physical);

  Page pages[1024];
};

Directory* current_directory = NULL;
Directory* kernel_directory = NULL;

namespace {

// TODO: Remove assumption of 16Mb
const uint32_t mem_end = 0x1000000;

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
    if (frames[i] == (uint32_t) -1)
      continue;
    for (uint32_t j = 0; j < 32; ++j) {
      uint32_t to_test = 0x1 << j;
      if ((frames[i] & to_test) == 0)
        return BIT_FROM_INDEX(i) + j;
    }
  }
  return (uint32_t) -1;
}

void SwitchPageDirectory(Directory* dir) {
  current_directory = dir;
  asm volatile("mov %0, %%cr3" : : "r" (dir->physicalAddress));
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

void PageFault(isr::Registers& regs) {
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
  screen::Printf(") at 0x%x - EIP: \n", faulting_address, regs.eip);
  PANIC("Page fault"); 
}

}  // namespace

void AllocFrame(Page* page, bool is_kernel, bool is_writeable) {
  // Check if already allocated
  if (page->frame != 0)
    return;

  uint32_t index = FindFrame();
  ASSERT(index != (uint32_t) -1);
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

Page* GetPage(uint32_t address, bool make, Directory* dir) {
  address = ADDR_TO_FRAME(address);
  uint32_t table_index = address / 1024;

  if (dir->tables[table_index] == NULL && make) {
    uint32_t tmp;
    void* table_mem = kalloc_pa(sizeof(Table), &tmp);
    dir->tables[table_index] = new (table_mem) Table();
    dir->physical[table_index] = tmp | 0x7;
  }

  ASSERT(dir->tables[table_index] != NULL);
  return &dir->tables[table_index]->pages[address%1024];
}

void Initialize() {
  num_frames = mem_end / 0x1000;
  frames = (uint32_t*) kalloc(INDEX_FROM_BIT(num_frames));
  memory::set(frames, 0, INDEX_FROM_BIT(num_frames));

  // Create the page directory
  void* kernel_directory_mem = kalloc_pa(sizeof(Directory));
  kernel_directory = new (kernel_directory_mem) Directory();
  kernel_directory->physicalAddress = (uint32_t) kernel_directory->physical;

  // Map pages in the kernel heap area.
  // Call GetPage but not AllocFrame. Tables can be created where necessary and
  // we can't allocate frames as they need to be identity mapped below.
  for (uint32_t i = KHEAP_START; i < KHEAP_END; i += 0x1000)
    GetPage(i, true, kernel_directory);

  // We need to identity map (phys addr = virt addr) from 0x0 to the end of
  // used memory, so we can access this transparently as if paging wasn't
  // enabled. Inside the loop body we actually change base_address by
  // calling kalloc().
  // Allocate a bit extra so the kernel heap can be initialised properly.
  for (uint32_t i = 0x0; i < 0x400000 /*base_address + 0x1000*/; i += 0x1000)
    AllocFrame(GetPage(i, true, kernel_directory), false, false);

  // Allocate the pages we mapped
  for (uint32_t i = KHEAP_START; i < KHEAP_END; i += 0x1000)
    AllocFrame(GetPage(i, true, kernel_directory), false, false);

  // TODO: interrupt constants
  isr::RegisterHandler(14, PageFault);

  SwitchPageDirectory(kernel_directory);

  // Create the kernel heap
  kheap = memory::Heap::Create(KHEAP_START, KHEAP_END, KHEAP_MAX, false, false);

  current_directory = kernel_directory->Clone();
  SwitchPageDirectory(current_directory);
}

void Shutdown() {
  memory::Heap::Destroy(kheap); 

  // TODO: free all directories
  kernel_directory->~Directory();
  kfree(kernel_directory);

  kfree(frames);

  isr::UnregisterHandler(14, PageFault);
}

uint32_t GetPhysicalAddress(uint32_t address) {
  Page* page = GetPage(address, false, kernel_directory);
  return FRAME_TO_ADDR(page->frame) + (address & 0x0FFF);
}

Directory::Directory() : physicalAddress(0) {
  memory::set((uint8_t*)tables, 0, sizeof(Table*) * ARRAY_SIZE(tables));
  memory::set((uint8_t*)physical, 0, sizeof(uint32_t) * ARRAY_SIZE(physical));
}

Directory::~Directory() {
  for (uint32_t i = 0; i < ARRAY_SIZE(tables); ++i) {
    if (tables[i] != 0) {
      tables[i]->~Table();
      kfree(tables[i]);
    }
  }
}

Directory* Directory::Clone() {
  uint32_t phys;
  void* dir_mem = kalloc_pa(sizeof(Directory), &phys);
  Directory* dir = new (dir_mem) Directory();

  // Get offset of physical table
  uint32_t offset = (uint32_t)dir->physical - (uint32_t) dir;
  dir->physicalAddress = phys + offset;

  // Copy the page table
  for (uint32_t i = 0; i < ARRAY_SIZE(tables); ++i) {
    if (tables[i] == 0)
      continue;
    if (kernel_directory->tables[i] == tables[i]) {
      // In the kernel - use the same pointer
      dir->tables[i] = tables[i];
      dir->physical[i] = physical[i];
    } else {
      // Copy the table
      uint32_t phys;
      dir->tables[i] = tables[i]->Clone(&phys);
      dir->physical[i] = phys | 0x7; // TODO: better flags
    }
  }

  return dir;
}

Table* Table::Clone(uint32_t* physical) {
  void* table_mem = kalloc_pa(sizeof(Table), physical);
  Table* table = new (table_mem) Table();

  for (uint32_t i = 0; i < ARRAY_SIZE(pages); ++i) {
    if (pages[i].frame == 0)
      continue;
    AllocFrame(&table->pages[i], false, false);
    // TODO: copy ctor
    table->pages[i].present = pages[i].present;
    table->pages[i].rw = pages[i].rw;
    table->pages[i].user = pages[i].user;
    table->pages[i].accessed = pages[i].accessed;
    table->pages[i].dirty = pages[i].dirty;
    // Copy the data across. See process.s.
    copy_page_physical(FRAME_TO_ADDR(pages[i].frame),
                       FRAME_TO_ADDR(table->pages[i].frame));
  }
  return table;
}

}  // namespace paging
