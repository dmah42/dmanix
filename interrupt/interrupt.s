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

ISR_NOERRCODE 0     ; div zero
ISR_NOERRCODE 1     ; debug
ISR_NOERRCODE 2     ; non maskable
ISR_NOERRCODE 3     ; breakpoint
ISR_NOERRCODE 4     ; into detected overflow
ISR_NOERRCODE 5     ; out of bounds
ISR_NOERRCODE 6     ; invalid opcode
ISR_NOERRCODE 7     ; no coprocessor
ISR_ERRCODE   8     ; double fault
ISR_NOERRCODE 9     ; coprocessor segment overrun
ISR_ERRCODE   10    ; bad tss
ISR_ERRCODE   11    ; segment not present
ISR_ERRCODE   12    ; stack fault
ISR_ERRCODE   13    ; general protection fault
ISR_ERRCODE   14    ; page fault
ISR_NOERRCODE 15    ; unknown interrupt
ISR_NOERRCODE 16    ; coprocessor fault
ISR_NOERRCODE 17    ; alignment check
ISR_NOERRCODE 18    ; machine check
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
ISR_NOERRCODE 128   ; syscall

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
