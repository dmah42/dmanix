#include "task.h"

#include <new>

#include "memory/kalloc.h"
#include "memory/memory.h"
#include "memory/paging.h"

#define KERNEL_STACK_SIZE 2048

// from boot.s
extern "C" uint32_t stack;

// from process.s
extern "C" uint32_t read_eip();

namespace dt {
extern void SetKernelStack(uint32_t stack);
}

namespace memory {
struct Page;

extern Directory* kernel_directory;
extern Directory* current_directory;

extern Page* GetPage(uint32_t address, bool make, Directory* dir);
extern void AllocFrame(Page* page, bool is_kernel, bool is_writeable);
}  // namespace memory

namespace task {
namespace {

struct Task {
  explicit Task(memory::Directory* directory)
      : pid(next_pid++),
        esp(0), ebp(0), eip(0),
        directory(directory),
        next(NULL) {
  }
  ~Task() {
    directory->~Directory();
    kfree(directory);
  }

  static uint32_t next_pid;

  uint32_t pid;
  uint32_t esp, ebp;
  uint32_t eip;
  uint32_t stack;  // kernel stack location
  memory::Directory* directory;
  Task* next;
};

Task* volatile current = NULL;
Task* volatile queue = NULL;

// static
uint32_t Task::next_pid = 1U;

void MoveStack(void* new_stack, uint32_t size) {
  for (uint32_t i = (uint32_t) new_stack;
       i >= (uint32_t) new_stack - size;
       i -= 0x1000) {
    memory::AllocFrame(
        memory::current_directory->GetPage(i, true), false, true);
  }

  // Flush the TLB
  uint32_t pd_addr;
  asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
  asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

  uint32_t old_stack_ptr;
  uint32_t old_base_ptr;
  asm volatile("mov %%esp, %0; \
                mov %%ebp, %1"
                : "=r" (old_stack_ptr), "=r" (old_base_ptr));

  uint32_t offset = (uint32_t) new_stack - stack;
  uint32_t new_stack_ptr = old_stack_ptr + offset;
  uint32_t new_base_ptr = old_base_ptr + offset;

  // copy the stack
  memory::copy8(reinterpret_cast<uint8_t*>(new_stack_ptr),
                reinterpret_cast<const uint8_t*>(old_stack_ptr),
                stack - old_stack_ptr);

  // Backtrace through the original stack, copying new values into the new
  // stack.
  for (uint32_t i = reinterpret_cast<uint32_t>(new_stack);
       i >= reinterpret_cast<uint32_t>(new_stack) - size;
       i -= sizeof(i)) {
    uint32_t tmp = *reinterpret_cast<uint32_t*>(i);
    // If the value of tmp is within the old stack, assume it's a base pointer
    // and remap it.
    if ((old_stack_ptr < tmp) && (tmp < stack)) {
      tmp += offset;
      uint32_t* tmp2 = reinterpret_cast<uint32_t*>(i);
      *tmp2 = tmp;
    }
  }

  // change stacks
  asm volatile("mov %0, %%esp; \
                mov %1, %%ebp"
                : : "r" (new_stack_ptr), "r" (new_base_ptr));
}

}  // namespace

void Initialize() {
  asm volatile("cli");

  MoveStack(reinterpret_cast<void*>(0xE0000000), 0x2000);

  // Initialize the first (kernel) task
  void* current_mem = kalloc(sizeof(Task));
  current = new (current_mem) Task(memory::current_directory);  // NOLINT
  current->stack = (uint32_t) kalloc_pa(KERNEL_STACK_SIZE);
  queue = current;

  asm volatile("sti");
}

void Shutdown() {
  Task* it = queue;
  while (it != NULL) {
    Task* next = it->next;
    it->~Task();
    kfree(it);
    it = next;
  }
}

uint32_t Fork() {
  asm volatile("cli");

  Task* parent = current;

  // Clone the address space
  memory::Directory* directory = memory::current_directory->Clone();

  // New process
  // TODO(dominic): clean these tasks up once they complete.
  void* task_mem = kalloc(sizeof(Task));
  Task* task = new (task_mem) Task(directory);  // NOLINT
  task->stack = (uint32_t) kalloc_pa(KERNEL_STACK_SIZE);

  // add to the end of the queue
  // TODO(dominic): track the end of the list
  Task* it = queue;
  while (it->next != NULL)
    it = it->next;
  it->next = task;

  uint32_t eip = read_eip();

  // We could be the parent or child here
  if (current == parent) {
    // we are the parent
    uint32_t esp, ebp;
    asm volatile("mov %%esp, %0;  \
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
  asm volatile("mov %%esp, %0;  \
                mov %%ebp, %1"
               : "=r" (esp), "=r" (ebp));

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
  if (current == NULL)
    current = queue;

  eip = current->eip;
  esp = current->esp;
  ebp = current->ebp;

  memory::current_directory = current->directory;

  dt::SetKernelStack(current->stack + KERNEL_STACK_SIZE);

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
          "r" (memory::current_directory->physicalAddress));
}

uint32_t PID() {
  return current->pid;
}

void UserMode() {
  dt::SetKernelStack(current->stack + KERNEL_STACK_SIZE);
  // Set up a stack structure for switching to user mode.
  // 0x23 is user mode data segment selector (0x20 | 0x3) and 0x1B is user mode
  // code segment selector (0x18 | 0x3).
  // push $1f pushes the address of the 1: label.
  // TODO(dominic): enable interrupts safely by replacing the sti with:
  //
  // pop %eax;
  // or %eax, $0x200;
  // push %eax;
  asm volatile("        \
      cli;              \
      mov $0x23, %ax;   \
      mov %ax, %ds;     \
      mov %ax, %es;     \
      mov %ax, %fs;     \
      mov %ax, %gs;     \
                        \
      sti;              \
      mov %esp, %eax;   \
      pushl $0x23;      \
      pushl %eax;       \
      pushf;            \
      pushl $0x1B;      \
      push $1f;         \
      iret;             \
    1:");
}

void KernelMode() {
  // WARNING: UNTESTED
  dt::SetKernelStack(current->stack);
  asm volatile("        \
      cli;              \
      mov $0x10, %ax;   \
      mov %ax, %ds;     \
      mov %ax, %es;     \
      mov %ax, %fs;     \
      mov %ax, %gs;     \
                        \
      mov %esp, %eax;   \
      pushl $0x10;      \
      pushl %eax;       \
      pushf;            \
      pushl $0x08;      \
      push $1f;         \
      iret;             \
    1:");
}

}  // namespace task
