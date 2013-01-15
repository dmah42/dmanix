#include "base/array_size.h"
#include "base/io.h"
#include "interrupt/isr.h"
#include "memory/memory.h"

// Defined in interrupt.s and irq.s
extern "C" {

extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);
extern void tss_flush();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

}

namespace dt {
namespace {

enum Segment {
  SEGMENT_NULL = 0,
  SEGMENT_CODE,
  SEGMENT_DATA,
  SEGMENT_USER_CODE,
  SEGMENT_USER_DATA,
  SEGMENT_TSS,
  NUM_SEGMENTS
};

struct GDTEntry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access_flags;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct GDTPointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct IDTEntry {
  uint16_t base_low;  // The lower 16 bits of the address to jump to
  uint16_t sel;       // kernel segment selector
  uint8_t always0;
  uint8_t flags;
  uint16_t base_high; // The high 16 bits of the address to jump to
} __attribute__((packed));

struct IDTPointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct TSSEntry {
  uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
  uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
  uint32_t ss0;        // The stack segment to load when we change to kernel mode.
  uint32_t esp1;       // Unused...
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;         // The value to load into ES when we change to kernel mode.
  uint32_t cs;         // The value to load into CS when we change to kernel mode.
  uint32_t ss;         // The value to load into SS when we change to kernel mode.
  uint32_t ds;         // The value to load into DS when we change to kernel mode.
  uint32_t fs;         // The value to load into FS when we change to kernel mode.
  uint32_t gs;         // The value to load into GS when we change to kernel mode.
  uint32_t ldt;        // Unused...
  uint16_t trap;
  uint16_t iomap_base;
} __attribute__((packed));

GDTEntry gdt_entries[NUM_SEGMENTS];
GDTPointer gdt_pointer;

IDTEntry idt_entries[256];
IDTPointer idt_pointer;

// Just use one TSS entry and use a software solution for switching.
TSSEntry tss_entry;

void SetGDTGate(Segment segment, uint32_t base, uint32_t limit,
                uint8_t access, uint8_t granularity) {
  gdt_entries[segment].base_low = (base & 0xFFFF);
  gdt_entries[segment].base_middle = (base >> 16) & 0xFF;
  gdt_entries[segment].base_high = (base >> 24) & 0xFF;

  gdt_entries[segment].limit_low = (limit & 0xFFFF);
  gdt_entries[segment].granularity = (limit >> 16) & 0xF;
  gdt_entries[segment].granularity |= granularity & 0xF0;
  gdt_entries[segment].access_flags = access;
}

void SetIDTGate(isr::Interrupt interrupt, uint32_t base, uint16_t sel,
                uint8_t flags) {
  idt_entries[interrupt].base_low = base & 0xFFFF;
  idt_entries[interrupt].base_high = (base >> 16) & 0xFFFF;

  idt_entries[interrupt].sel = sel;
  idt_entries[interrupt].always0 = 0;

  idt_entries[interrupt].flags = flags | 0x60;
}

void WriteTSS(uint16_t ss0, uint32_t esp0) {
  uint32_t base = (uint32_t) &tss_entry;
  uint32_t limit = base + sizeof(TSSEntry);

  SetGDTGate(SEGMENT_TSS, base, limit, 0xE9, 0x00);
  memory::set(&tss_entry, 0, sizeof(TSSEntry));

  tss_entry.ss0 = ss0;
  tss_entry.esp0 = esp0;

  // Set cs, ss, ds, es, fs, gs entries. These specify the segments to be loaded
  // when the processor switches to kernel mode. Therefore, they're the normal
  // kernel code/data segments (0x08, 0x10) but with the last two bits set (|
  // 0x3).
  tss_entry.cs = 0x08 | 0x03;
  tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x10 | 0x3;
}

void InitGDT() {
  gdt_pointer.limit = (sizeof(GDTEntry) * NUM_SEGMENTS) - 1;
  gdt_pointer.base = (uint32_t)&gdt_entries;

  SetGDTGate(SEGMENT_NULL,      0,          0,    0,    0);
  // TODO: set the flags more explicitly
  SetGDTGate(SEGMENT_CODE,      0, 0xFFFFFFFF, 0x9A, 0xCF);
  SetGDTGate(SEGMENT_DATA,      0, 0xFFFFFFFF, 0x92, 0xCF);
  SetGDTGate(SEGMENT_USER_CODE, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  SetGDTGate(SEGMENT_USER_DATA, 0, 0xFFFFFFFF, 0xF2, 0xCF);
  WriteTSS(0x10, 0x0);

  gdt_flush((uint32_t)&gdt_pointer);
  tss_flush();
}

const uint16_t MASTER_PIC = 0x20;
const uint16_t SLAVE_PIC = 0xA0;
const uint16_t MASTER_PIC_COMMAND = MASTER_PIC;
const uint16_t MASTER_PIC_DATA = MASTER_PIC + 1;
const uint16_t SLAVE_PIC_COMMAND = SLAVE_PIC;
const uint16_t SLAVE_PIC_DATA = SLAVE_PIC + 1;

void RemapPIC() {
  // Initialize PICs
  io::outb(MASTER_PIC_COMMAND, 0x11);
  io::outb(SLAVE_PIC_COMMAND, 0x11);

  // Set offsets
  io::outb(MASTER_PIC_DATA, 0x20);
  io::outb(SLAVE_PIC_DATA, 0x28);

  // Set connections
  io::outb(MASTER_PIC_DATA, 0x4);
  io::outb(SLAVE_PIC_DATA, 0x2);

  // Set 8086 mode
  io::outb(MASTER_PIC_DATA, 0x1);
  io::outb(SLAVE_PIC_DATA, 0x1);

  // Complete
  io::outb(MASTER_PIC_DATA, 0x0);
  io::outb(SLAVE_PIC_DATA, 0x0);
}

void SetPICGates() {
  SetIDTGate(IRQ0, (uint32_t)irq0, 0x08, 0x8E);
  SetIDTGate(IRQ1, (uint32_t)irq1, 0x08, 0x8E);
  SetIDTGate(IRQ2, (uint32_t)irq2, 0x08, 0x8E);
  SetIDTGate(IRQ3, (uint32_t)irq3, 0x08, 0x8E);
  SetIDTGate(IRQ4, (uint32_t)irq4, 0x08, 0x8E);
  SetIDTGate(IRQ5, (uint32_t)irq5, 0x08, 0x8E);
  SetIDTGate(IRQ6, (uint32_t)irq6, 0x08, 0x8E);
  SetIDTGate(IRQ7, (uint32_t)irq7, 0x08, 0x8E);
  SetIDTGate(IRQ8, (uint32_t)irq8, 0x08, 0x8E);
  SetIDTGate(IRQ9, (uint32_t)irq9, 0x08, 0x8E);
  SetIDTGate(IRQ10, (uint32_t)irq10, 0x08, 0x8E);
  SetIDTGate(IRQ11, (uint32_t)irq11, 0x08, 0x8E);
  SetIDTGate(IRQ12, (uint32_t)irq12, 0x08, 0x8E);
  SetIDTGate(IRQ13, (uint32_t)irq13, 0x08, 0x8E);
  SetIDTGate(IRQ14, (uint32_t)irq14, 0x08, 0x8E);
  SetIDTGate(IRQ15, (uint32_t)irq15, 0x08, 0x8E);
}

void InitIDT() {
  idt_pointer.limit = (sizeof(IDTEntry) * ARRAY_SIZE(idt_entries)) - 1;
  idt_pointer.base = (uint32_t)&idt_entries;

  memory::set(idt_entries, 0, sizeof(IDTEntry) * ARRAY_SIZE(idt_entries));

  // Remap IRQ table to avoid conflicts
  RemapPIC();

  // TODO: better way
  SetIDTGate(0,  (uint32_t)isr0, 0x08, 0x8E);
  SetIDTGate(1,  (uint32_t)isr1, 0x08, 0x8E);
  SetIDTGate(2,  (uint32_t)isr2, 0x08, 0x8E);
  SetIDTGate(3,  (uint32_t)isr3, 0x08, 0x8E);
  SetIDTGate(4,  (uint32_t)isr4, 0x08, 0x8E);
  SetIDTGate(5,  (uint32_t)isr5, 0x08, 0x8E);
  SetIDTGate(6,  (uint32_t)isr6, 0x08, 0x8E);
  SetIDTGate(7,  (uint32_t)isr7, 0x08, 0x8E);
  SetIDTGate(8,  (uint32_t)isr8, 0x08, 0x8E);
  SetIDTGate(9,  (uint32_t)isr9, 0x08, 0x8E);
  SetIDTGate(10, (uint32_t)isr10, 0x08, 0x8E);
  SetIDTGate(11, (uint32_t)isr11, 0x08, 0x8E);
  SetIDTGate(12, (uint32_t)isr12, 0x08, 0x8E);
  SetIDTGate(13, (uint32_t)isr13, 0x08, 0x8E);
  SetIDTGate(14, (uint32_t)isr14, 0x08, 0x8E);
  SetIDTGate(15, (uint32_t)isr15, 0x08, 0x8E);
  SetIDTGate(16, (uint32_t)isr16, 0x08, 0x8E);
  SetIDTGate(17, (uint32_t)isr17, 0x08, 0x8E);
  SetIDTGate(18, (uint32_t)isr18, 0x08, 0x8E);
  SetIDTGate(19, (uint32_t)isr19, 0x08, 0x8E);
  SetIDTGate(20, (uint32_t)isr20, 0x08, 0x8E);
  SetIDTGate(21, (uint32_t)isr21, 0x08, 0x8E);
  SetIDTGate(22, (uint32_t)isr22, 0x08, 0x8E);
  SetIDTGate(23, (uint32_t)isr23, 0x08, 0x8E);
  SetIDTGate(24, (uint32_t)isr24, 0x08, 0x8E);
  SetIDTGate(25, (uint32_t)isr25, 0x08, 0x8E);
  SetIDTGate(26, (uint32_t)isr26, 0x08, 0x8E);
  SetIDTGate(27, (uint32_t)isr27, 0x08, 0x8E);
  SetIDTGate(28, (uint32_t)isr28, 0x08, 0x8E);
  SetIDTGate(29, (uint32_t)isr29, 0x08, 0x8E);
  SetIDTGate(30, (uint32_t)isr30, 0x08, 0x8E);
  SetIDTGate(31, (uint32_t)isr31, 0x08, 0x8E);
  SetIDTGate(128, (uint32_t)isr128, 0x08, 0x8E);

  SetPICGates();

  idt_flush((uint32_t)&idt_pointer);
}

}  // namespace

void SetKernelStack(uint32_t stack) {
  tss_entry.esp0 = stack;
}

void Initialize() {
  InitGDT();
  InitIDT();
}

}  // namespace dt
