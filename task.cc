#include "base.h"
#include "kalloc.h"
#include "memory.h"
#include "paging.h"

// from boot.s
extern "C" uint32_t stack;

// from process.s
extern "C" uint32_t read_eip();

namespace paging {
struct Page;

extern Directory* kernel_directory;
extern Directory* current_directory;

extern Page* GetPage(uint32_t address, bool make, Directory* dir);
extern void AllocFrame(Page* page, bool is_kernel, bool is_writeable);
}

namespace task {
namespace {

struct Task {
  uint32_t pid;
  uint32_t esp, ebp;
  uint32_t eip;
  paging::Directory* directory;
  Task* next;
};

Task* volatile current;
Task* volatile queue;

uint32_t next_pid = 1;

void MoveStack(void* new_stack, uint32_t size) {
  for (uint32_t i = (uint32_t) new_stack;
       i >= (uint32_t) new_stack - size;
       i -= 0x1000) {
    AllocFrame(GetPage(i, true, paging::current_directory), false, true);
  }

  // Flush the TLB
  uint32_t pd_addr;
  asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
  asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

  uint32_t old_stack_ptr;
  asm volatile("mov %%esp, %0" : "=r" (old_stack_ptr));
  uint32_t old_base_ptr;
  asm volatile("mov %%ebp, %0" : "=r" (old_base_ptr));

  uint32_t offset = (uint32_t) new_stack - stack;
  uint32_t new_stack_ptr = old_stack_ptr + offset;
  uint32_t new_base_ptr = old_base_ptr + offset;

  // copy the stack
  memory::copy((void*) new_stack_ptr, (void*) old_stack_ptr,
               stack - old_stack_ptr);
  
  // Backtrace through the original stack, copying new values into the new
  // stack.
  for (uint32_t i = (uint32_t) new_stack; i >= (uint32_t) new_stack - size;
       i -= 4) {
    uint32_t tmp = *(uint32_t*) i;
    // If the value of tmp is within the old stack, assume it's a base pointer
    // and remap it.
    if ((old_stack_ptr < tmp) && (tmp < stack)) {
      tmp += offset;
      uint32_t* tmp2 = (uint32_t*) i;
      *tmp2 = tmp;
    }
  }

  // change stacks
  asm volatile("mov %0, %%esp" : : "r" (new_stack_ptr));
  asm volatile("mov %0, %%ebp" : : "r" (new_base_ptr));
}

}  // namespace

void Initialize() {
  asm volatile("cli");

  MoveStack((void*) 0xE0000000, 0x4000);

  // Initialize the first (kernel) task
  current = queue = (Task*) kalloc(sizeof(Task));
  current->pid = next_pid++;
  current->esp = current->ebp = 0;
  current->eip = 0;
  current->directory = paging::current_directory;
  current->next = 0;

  asm volatile("sti");
}

uint32_t Fork() {
  asm volatile("cli");

  Task* parent = current;

  // Clone the address space
  paging::Directory* directory = paging::current_directory->Clone();

  // New process
  // TODO: ctor
  Task* task = (Task*) kalloc(sizeof(Task));
  task->pid = next_pid++;
  task->esp = task->ebp = 0;
  task->eip = 0;
  task->directory = directory;
  // TODO: NULL
  task->next = 0;

  // add to the end of the queue
  // TODO: track the end of the list
  Task* it = (Task*) queue;
  while (it->next != 0)
    it = it->next;
  it->next = task;

  uint32_t eip = read_eip();

  // We could be the parent or child here
  if (current == parent) {
    // we are the parent
    uint32_t esp, ebp;
    asm volatile(
        "mov %%esp, %0;  \
         mov %%ebp, %1"
        : "=r" (esp), "=r" (ebp));
    task->esp = esp;
    task->ebp = ebp;
    task->eip = eip;

    asm volatile("sti");
    return task->pid;
  }
  return 0;
}

void Switch() {
  // Uninitialized - early return
  if (current == 0)
    return;

  uint32_t esp, ebp;
  asm volatile(
      "mov %%esp, %0;  \
       mov %%ebp, %1"
      : "=r" (ebp), "=r" (esp));

  // Read the instruction pointer.
  // Once this method exits, we could be in one of two states:
  //   1. we called the method and it returned the eip
  //   2. we just switched tasks and it seems as if read_eip just returned.
  // In the second case, we need to immediately return. To detect it, we put a
  // dummy value in eax further down at the end of this method. As returns are
  // placed in eax it will appear as if eip is 0x12345.
  uint32_t eip = read_eip();

  if (eip == 0x12345)
    return;

  current->eip = eip;
  current->esp = esp;
  current->ebp = ebp;

  // get the next task
  current = current->next;
  if (current == 0)
    current = queue;

  esp = current->esp;
  ebp = current->ebp;

  // - Stop interrupts
  // - Put eip into ecx
  // - load stack and base from the new task
  // - change directory to the physical address of the new directory
  // - put a dummy value into eax so we recognize switched task
  // - restart interrupts
  // - jump to ecx (new eip)
  asm volatile("            \
      cli;                  \
      mov %0, %%ecx;        \
      mov %1, %%esp;        \
      mov %2, %%ebp;        \
      mov %3, %%cr3;        \
      mov $0x12345, %%eax;  \
      sti;                  \
      jmp *%%ecx"
      : : "r" (eip),
          "r" (esp),
          "r" (ebp),
          "r" (paging::current_directory->physicalAddress));
}

uint32_t PID() {
  return current->pid;
}

}  // namespace task
