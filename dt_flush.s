; global descriptor table and interrupt descriptor table setup code.

[GLOBAL gdt_flush]    ; expose gdt_flush to C++

gdt_flush:
  mov   eax, [esp+4]  ; Get the pointer to the GDT
  lgdt  [eax]         ; and load it

  mov   ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
  mov   ds, ax        ; Load all data segment selectors
  mov   es, ax
  mov   fs, ax
  mov   gs, ax
  mov   ss, ax
  jmp   0x08:.flush   ; 0x08 is the offset to our code segment
.flush:
  ret

[GLOBAL idt_flush]    ; expose idt_flush to C++

idt_flush:
  mov   eax, [esp+4]
  lidt  [eax]
  ret
