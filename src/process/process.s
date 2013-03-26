global copy_page_physical
copy_page_physical:
  push  ebx             ; according to __cdecl, we must preserve the contents
                        ; of ebx
  pushf                 ; push eflags so we can pop it and reenable
                        ; interrupts later, if they were enabled
  cli                   ; disable interrupts
  mov   ebx, [esp + 12] ; source address
  mov   ecx, [esp + 16] ; destination address

  mov   edx, cr0        ; control register
  and   edx, 0x7FFFFFFF ; and
  mov   cr0, edx        ; disable paging

  mov   edx, 1024       ; 1024 * 4 bytes = 4096 bytes to copy

.loop:
  mov   eax, [ebx]      ; get the word at the source address
  mov   [ecx], eax      ; store it at the destination address
  add   ebx, 4          ; source address += sizeof(word)
  add   ecx, 4          ; destination address += sizeof(word)
  dec   edx             ; one fewer word to copy
  jnz   .loop

  mov   edx, cr0        ; get the control register
  or    edx, 0x80000000 ; and
  mov   cr0, edx        ; enable paging

  popf                  ; pop eflags
  pop   ebx             ; get the original ebx back
  ret

global read_eip
read_eip:
  pop   eax
  jmp   eax

global flush_page_directory
flush_page_directory:
  mov   eax, cr3
  mov   cr3, eax
  ret

extern resolveSymbol
global resolveSymbol_asm
resolveSymbol_asm:
  call resolveSymbol
  pop ecx               ; Remove library ID parameter
  pop ecx               ; Remove symbol offset parameter
  jmp eax
