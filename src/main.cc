#include "dt.h"
#include "fs/initrd.h"
#include "fs/fs.h"
#include "memory/heap.h"
#include "memory/paging.h"
#include "multiboot.h"
#include "screen.h"
#include "syscall.h"
#include "task.h"
#include "test.h"
#include "timer.h"

// from kalloc.cc
extern uint32_t base_address;

// from boot.s
extern "C" multiboot::Info* mbd;

namespace fs {
extern Node* root;
}

namespace memory {
// from paging.cc
extern Heap* kheap;

// from kalloc.cc
extern uint32_t base_address;
}

int main() {
  dt::Initialize();
  screen::Clear();

  asm volatile("sti");
  timer::Initialize(50);

  ASSERT(mbd->mods_count > 0);
  const multiboot::Module* mod = (multiboot::Module*) mbd->mods_addr;

  // Update base address so we don't trample the modules.
  memory::base_address = mod[mbd->mods_count - 1].end_address;

  // test::memory();
  memory::Initialize(mbd->mem_upper);

  task::Initialize();

  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts("Welcome to ");
  screen::SetColor(COLOR_BLUE, COLOR_BLACK);
  screen::puts("DMA");
  screen::SetColor(COLOR_GREEN, COLOR_BLACK);
  screen::puts("NIX\n");
  screen::ResetColor();

  fs::root = initrd::Initialize(mod[0]);

  syscall::Initialize();

#ifdef DEBUG
  multiboot::Dump();
#endif
  // test::vga();
  // test::colors();
  // test::interrupt();
  // test::timer();
  // test::page_fault();
  // test::memory();
  // test::initrd();
  // test::fork();
  // test::user_mode();
  // test::mode13();

  // TODO: Launch executables

  screen::puts("Shutting down... ");
  syscall::Shutdown();
  initrd::Shutdown();
  timer::Shutdown();
  memory::Shutdown();
  screen::puts("done.\n");

  // kheap->Dump();

  screen::SetColor(COLOR_WHITE, COLOR_BLACK);
  screen::puts("Goodbye.\n");

  // Loop forever instead of exiting to halt.
  for (;;);
  return 0;
}
