; Interrupt service routine wrappers.

; Stub for ISR which adds dummy error code byte
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push  byte 0
    push  byte %1
    jmp   isr_common_stub
%endmacro

; Stub for ISR that adds its own error code
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push  byte %1
    jmp   isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

extern isr_handler

; Common ISR stub. Save processor state, set up for kernel mode, call the
; C-level fault handler, and restore state.
isr_common_stub:
  pusha             ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

  mov   ax, ds
  push  eax         ; save the data segment descriptor

  mov   ax, 0x10    ; load the kernel data segment descriptor
  mov   ds, ax
  mov   es, ax
  mov   fs, ax
  mov   gs, ax

  call  isr_handler

  pop   ebx         ; reload the original data segment descriptor
  mov   ds, bx
  mov   es, bx
  mov   fs, bx
  mov   gs, bx

  popa              ; Pops edi,esi, ...
  add   esp, 8      ; Clean up the error code and ISR number
  sti
  iret              ; Pops five things: cs, eip, eflags, ss, and esp
