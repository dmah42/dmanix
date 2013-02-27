#include "paging.h"

#include <new>

#include "base/array_size.h"
#include "bitset.h"
#include "interrupt/isr.h"
#include "memory/heap.h"
#include "memory/memory.h"
#include "screen.h"

// from process.s
extern "C" void copy_page_physical(uint32_t dest, uint32_t src);

#define KHEAP_START         0xC0000000
// TODO: Get this from initrd config
#define KHEAP_INITIAL_SIZE  0x00100000  // 1Mb Kernel heap
#define KHEAP_END           0xCFFFF000

#define UHEAP_START         0xD0000000
#define UHEAP_INITIAL_SIZE  0x00400000  // 4Mb User heap
#define UHEAP_END           0xDFFFF000           

#define PAGE_SIZE           0x1000

namespace memory {

// from kalloc.cc
extern uint32_t base_address;

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
    memory::set8((uint8_t*)pages, 0, sizeof(Page) * ARRAY_SIZE(pages));
  }
  // TODO: copy ctor
  Table* Clone(uint32_t* physical);

  Page pages[1024];
};

Directory* current_directory = NULL;
Directory* kernel_directory = NULL;

Heap* kheap = NULL;
Heap* uheap = NULL;

namespace {

// bitset of used/free frames
bitset* frames;

#define FRAME_TO_ADDR(f)    (f*0x1000)
#define ADDR_TO_FRAME(a)    (a/0x1000)

void SwitchPageDirectory(Directory* dir) {
  current_directory = dir;
  asm volatile("mov %0, %%cr3" : : "r" (dir->physicalAddress));
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r" (cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

// TODO: mode 13 version
void PageFault(isr::Registers* regs) {
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  bool present = !(regs->err_code & 0x1);
  bool rw = regs->err_code & 0x2;
  bool user = regs->err_code & 0x4;
  bool reserved = regs->err_code & 0x8;
  bool instruction = regs->err_code & 0x10;

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
  screen::Printf(") at 0x%x - EIP: \n", faulting_address, regs->eip);
  PANIC("Page fault"); 
}

}  // namespace

void AllocFrame(Page* page, bool is_kernel, bool is_writeable) {
  // Check if already allocated
  if (page->frame != 0)
    return;

  uint32_t index = frames->next();
  ASSERT(index != bitset::NOT_FOUND);
  frames->set(index);
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

  frames->clear(ADDR_TO_FRAME(frame));
  page->frame = 0;
}

void Initialize(uint32_t mem_end) {
  mem_end *= 1024;
  // Align mem_end to PAGE_SIZE
  mem_end -= mem_end % PAGE_SIZE;
  screen::Printf("%d MB RAM detected.\n", mem_end / (1024 * 1024));
  uint32_t num_frames = mem_end / PAGE_SIZE;
  void* frames_mem = kalloc(sizeof(bitset));
  frames = new (frames_mem) bitset(num_frames);

  // Create the page directory
  void* kernel_directory_mem = kalloc_pa(sizeof(Directory));
  kernel_directory = new (kernel_directory_mem) Directory();

  // Map pages in the kernel heap area.
  // Call GetPage but not AllocFrame. Tables can be created where necessary and
  // we can't allocate frames as they need to be identity mapped below.
  for (uint32_t i = KHEAP_START; i < KHEAP_END; i += PAGE_SIZE)
    kernel_directory->GetPage(i, true);

  for (uint32_t i = UHEAP_START; i < UHEAP_END; i += PAGE_SIZE)
    kernel_directory->GetPage(i, true);

  // We need to identity map (phys addr = virt addr) from 0x0 to the end of
  // used memory, so we can access this transparently as if paging wasn't
  // enabled. Inside the loop body we actually change base_address by
  // calling kalloc().
  // Allocate a bit extra so the kernel heap can be initialised properly.
  for (uint32_t i = 0x0; i < base_address + PAGE_SIZE; i += PAGE_SIZE)
    AllocFrame(kernel_directory->GetPage(i, true), false, false);

  // Allocate the pages we mapped
  for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE;
       i += PAGE_SIZE) {
    AllocFrame(kernel_directory->GetPage(i, true), false, false);
  }

  for (uint32_t i = UHEAP_START; i < UHEAP_START + UHEAP_INITIAL_SIZE;
       i += PAGE_SIZE) {
    AllocFrame(kernel_directory->GetPage(i, true), false, true);
  }

  // TODO: interrupt constants
  isr::RegisterHandler(14, PageFault);

  SwitchPageDirectory(kernel_directory);

  // Create the kernel heap
  kheap = memory::Heap::Create(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE,
                               KHEAP_END, false, false);
  uheap = memory::Heap::Create(UHEAP_START, UHEAP_START + UHEAP_INITIAL_SIZE,
                               UHEAP_END, false, false);

  current_directory = kernel_directory->Clone();
  SwitchPageDirectory(current_directory);
}

void Shutdown() {
  memory::Heap::Destroy(uheap);
  uheap = NULL;

  memory::Heap::Destroy(kheap); 
  kheap = NULL;

  // TODO: free all directories
  kernel_directory->~Directory();
  kfree(kernel_directory);

  frames->~bitset();
  kfree(frames);

  isr::UnregisterHandler(14, PageFault);
}

uint32_t GetPhysicalAddress(uint32_t address) {
  Page* page = current_directory->GetPage(address, false);
  return FRAME_TO_ADDR(page->frame) + (address & 0x0FFF);
}

Directory::Directory() : physicalAddress(0) {
  memory::set8((uint8_t*)tables, 0, sizeof(Table*) * ARRAY_SIZE(tables));
  memory::set8((uint8_t*)physical, 0, sizeof(uint32_t) * ARRAY_SIZE(physical));
  physicalAddress = (uint32_t) physical;
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

Page* Directory::GetPage(uint32_t address, bool make) {
  address = ADDR_TO_FRAME(address);
  uint32_t table_index = address / 1024;

  if (tables[table_index] == NULL && make) {
    uint32_t tmp;
    void* table_mem = kalloc_pa(sizeof(Table), &tmp);
    tables[table_index] = new (table_mem) Table();
    physical[table_index] = tmp | 0x7;
  }

  ASSERT(tables[table_index] != NULL);
  return &tables[table_index]->pages[address%1024];
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

}  // namespace memory
