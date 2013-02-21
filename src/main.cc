#include "dt.h"
#include "fs/initrd.h"
#include "fs/fs.h"
#include "fs/node.h"
#include "memory/heap.h"
#include "memory/paging.h"
#include "multiboot.h"
#include "screen.h"
#include "syscall.h"
#include "task.h"
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

namespace test {

  void fork() {
    // create a new process in a new address space which is a clone
    uint32_t ret = task::Fork();
    screen::Printf("Fork returned 0x%x and getpid() returned 0x%x\n",
                   ret, task::PID());
    screen::puts("=========================================\n");
  }

  void vga() {
    screen::puts("0         1         2         3         4         5         6         7       \n");
    screen::puts("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
    screen::puts("2\n");
    screen::puts("3\n");
    screen::puts("4\n");
    screen::puts("5\n");
    screen::puts("6\n");
    screen::puts("7\n");
    screen::puts("8\n");
    screen::puts("9\n");
    screen::puts("10\n");
    screen::puts("11\n");
    screen::puts("12\n");
    screen::puts("13\n");
    screen::puts("14\n");
    screen::puts("15\n");
    screen::puts("16\n");
    screen::puts("17\n");
    screen::puts("18\n");
    screen::puts("19\n");
    screen::puts("20\n");
    screen::puts("21\n");
    screen::puts("22\n");
    screen::puts("23\n");
    screen::puts("24\n");
    screen::puts("25");
  }

  void colors() {
    screen::puts("foreground:\n");
    screen::SetColor(COLOR_BLACK, COLOR_BLACK);
    screen::puts("black\t");
    screen::SetColor(COLOR_DARK_RED, COLOR_BLACK);
    screen::puts("dark red\t");
    screen::SetColor(COLOR_DARK_GREEN, COLOR_BLACK);
    screen::puts("dark green\t");
    screen::SetColor(COLOR_DARK_YELLOW, COLOR_BLACK);
    screen::puts("dark yellow\n");
    screen::SetColor(COLOR_DARK_BLUE, COLOR_BLACK);
    screen::puts("dark blue\t");
    screen::SetColor(COLOR_DARK_MAGENTA, COLOR_BLACK);
    screen::puts("dark magenta\t");
    screen::SetColor(COLOR_DARK_CYAN, COLOR_BLACK);
    screen::puts("dark cyan\t");
    screen::SetColor(COLOR_LIGHT_GREY, COLOR_BLACK);
    screen::puts("light grey\n");
    screen::SetColor(COLOR_GREY, COLOR_BLACK);
    screen::puts("grey\t");
    screen::SetColor(COLOR_RED, COLOR_BLACK);
    screen::puts("red\t");
    screen::SetColor(COLOR_GREEN, COLOR_BLACK);
    screen::puts("green\t");
    screen::SetColor(COLOR_YELLOW, COLOR_BLACK);
    screen::puts("yellow\n");
    screen::SetColor(COLOR_BLUE, COLOR_BLACK);
    screen::puts("blue\t");
    screen::SetColor(COLOR_MAGENTA, COLOR_BLACK);
    screen::puts("magenta\t");
    screen::SetColor(COLOR_CYAN, COLOR_BLACK);
    screen::puts("cyan\t");
    screen::SetColor(COLOR_WHITE, COLOR_BLACK);
    screen::puts("white\n");

    screen::puts("background:\n");
    screen::SetColor(COLOR_BLACK, COLOR_BLACK);
    screen::puts("black\t");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_RED);
    screen::puts("dark red\t");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_GREEN);
    screen::puts("dark green\t");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_YELLOW);
    screen::puts("dark yellow\n");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_BLUE);
    screen::puts("dark blue\t");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_MAGENTA);
    screen::puts("dark magenta\t");
    screen::SetColor(COLOR_BLACK, COLOR_DARK_CYAN);
    screen::puts("dark cyan\t");
    screen::SetColor(COLOR_BLACK, COLOR_LIGHT_GREY);
    screen::puts("light grey\n");
    screen::SetColor(COLOR_BLACK, COLOR_GREY);
    screen::puts("grey\t");
    screen::SetColor(COLOR_BLACK, COLOR_RED);
    screen::puts("red\t");
    screen::SetColor(COLOR_BLACK, COLOR_GREEN);
    screen::puts("green\t");
    screen::SetColor(COLOR_BLACK, COLOR_YELLOW);
    screen::puts("yellow\n");
    screen::SetColor(COLOR_BLACK, COLOR_BLUE);
    screen::puts("blue\t");
    screen::SetColor(COLOR_BLACK, COLOR_MAGENTA);
    screen::puts("magenta\t");
    screen::SetColor(COLOR_BLACK, COLOR_CYAN);
    screen::puts("cyan\t");
    screen::SetColor(COLOR_BLACK, COLOR_WHITE);
    screen::puts("white\n");
  }

  void mode13() {
    screen::Mode13h();

    for (uint16_t x = 0; x < 320; ++x)
      for (uint16_t y = 0; y < 200; ++y)
        screen::Pixel(x, y, COLOR_DARK_BLUE);
    screen::Line(0, 0, 320, 200, COLOR_WHITE);
    for(;;);
  }

  void interrupt() {
    asm volatile("int $0x3");
    asm volatile("int $0x4");
  }

  void timer_callback(uint32_t tick) {
    screen::Printf("Tick: %u\n", tick);
  }

  void timer() {
    timer::RegisterCallback(timer_callback);
  }

  void page_fault() {
    uint32_t* ptr = (uint32_t*)0xA0000000;
    uint32_t do_page_fault = *ptr;
    (void) do_page_fault;
  }

  void memory() {
    screen::puts("memory test start\n");
    void* a = kalloc(8);
    screen::Printf("a: 0x%x", a);
    void* b = kalloc(8);
    screen::Printf(", b: 0x%x\n", b);

    if (memory::kheap)
      memory::kheap->Dump();

    screen::puts("about to free\n");
    kfree(b);
    kfree(a);
    screen::puts("freed\n");
    void* d = kalloc(12);
    screen::Printf("d: 0x%x\n", d);
    screen::puts("freeing...\n");
    kfree(d); 
    screen::puts("freed\n");
  }

  void initrd() {
    asm volatile("cli");
    uint32_t i = 0;
    fs::DirEntry* node = NULL;
    while ((node = fs::root->ReadDir(i)) != NULL) {
      screen::Printf("Found file: %s", node->name);
      fs::Node* fsnode = fs::root->FindDir(node->name);
      if ((fsnode->flags & fs::FLAG_DIRECTORY) == fs::FLAG_DIRECTORY)
        screen::puts("\t(directory)\n");
      else {
        screen::puts("\tcontents:\n\t\"");
        char buf[256];
        fsnode->Read(0, sizeof(buf), (uint8_t*) buf);
        screen::puts(buf);
        screen::puts("\"\n");
      }
      ++i;
    }
    screen::puts("end of initrd\n");
    asm volatile("sti");
  }

  void user_mode() {
    task::UserMode();
    syscall::_puts("User mode syscall\n");
  }
}  // namespace test

int main() {
  dt::Initialize();
  screen::Clear();

  asm volatile("sti");
  timer::Initialize(50);

  //multiboot::Dump();

  ASSERT(mbd->mods_count > 0);
  const multiboot::Module* mod = (multiboot::Module*) mbd->mods_addr;

  // Update base address so we don't trample the modules.
  memory::base_address = mod[mbd->mods_count - 1].end_address;

  // test::memory();
  memory::Initialize();

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
