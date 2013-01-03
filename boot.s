MBOOT_PAGE_ALIGN    equ 1 << 0  ; load kernel and modules on page boundary
MBOOT_MEM_INFO      equ 1 << 1  ; provide kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

[GLOBAL mboot]      ; mboot accessible from C
[EXTERN code]       ; start of .text
[EXTERN bss]        ; start of .bss
[EXTERN end]        ; end of loadable sections

mboot:
  dd MBOOT_HEADER_MAGIC
  dd MBOOT_HEADER_FLAGS
  dd MBOOT_CHECKSUM

  dd mboot
  dd code
  dd bss
  dd end
  dd start

[GLOBAL start]      ; kernel entry point
[EXTERN main]       ; entry point of C++ code

start:
  ;push ebx          ; load multiboot header location

  ; execute the kernel
  cli               ; disable interrupts
  call main
  jmp $             ; infinite loop
