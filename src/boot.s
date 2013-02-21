global start
global mbmagic
global mbd
global stack

extern main

extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors

; set up multiboot header
MBOOT_PAGE_ALIGN  equ 1 << 0  ; load kernel and modules on page boundary
MBOOT_MEM_INFO    equ 1 << 1  ; provide kernel with memory info
MBOOT_MAGIC       equ 0x1BADB002
MBOOT_FLAGS       equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM    equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .text

align 4
  dd MBOOT_MAGIC
  dd MBOOT_FLAGS
  dd MBOOT_CHECKSUM

start:
  mov   [mbmagic], eax
  mov   [mbd], ebx
  mov   [stack], esp

  ; call constructors
  mov   ebx, start_ctors
  jmp   .ctors_until_end
.call_ctor:
  call  [ebx]
  add   ebx, 4
.ctors_until_end:
  cmp   ebx, end_ctors
  jb    .call_ctor

  ; call kernel
  cli
  call main

  ; call destructors in reverse order
  mov   ebx, end_dtors
  jmp   .dtors_until_end
.call_dtor:
  sub   ebx, 4
  call  [ebx]
.dtors_until_end:
  cmp   ebx, start_dtors
  ja    .call_dtor

  cli
.hang:
  hlt
  jmp   .hang

section .bss

align 4
mbmagic:  resd 1
mbd:      resd 1
stack:    resd 1
