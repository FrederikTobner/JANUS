# Boot Entry Assembly

GRUB finds our Multiboot header, validates it, and jumps to our entry point. But there's a problem: GRUB leaves the CPU in an undefined state. Interrupts might be enabled, the stack pointer is garbage, and the CPU is in 32-bit mode.

We need assembly code to set up a minimal runtime environment before we can execute C code.

## The Boot Handoff

GRUB boots us in 32-bit protected mode, but our kernel is compiled for 64-bit. We need to:

1. Start in 32-bit mode (as GRUB gives us)
2. Set up page tables for identity mapping
3. Enable PAE (Physical Address Extension) and long mode
4. Load a 64-bit GDT (Global Descriptor Table)
5. Jump to 64-bit code
6. Call our 64-bit kernel

Create `boot/boot.asm`:

```nasm
; Boot entry point - called by GRUB in 32-bit protected mode
global _start
extern kernel_main

; Reserve stack space in BSS section
section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

; Page tables for long mode (must be page-aligned)
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

section .text
bits 32                     ; GRUB puts us in 32-bit protected mode

_start:
    ; At this point:
    ; - EAX = multiboot magic (0x36d76289)
    ; - EBX = physical address of multiboot info structure
    ; - CPU is in 32-bit protected mode
    
    ; Save multiboot info (we'll need them after switching to long mode)
    mov edi, eax            ; Save magic
    mov esi, ebx            ; Save multiboot info pointer
    
    ; Set up stack pointer
    mov esp, stack_top
    
    ; Set up page tables and enable long mode
    call setup_page_tables
    call enable_paging
    
    ; Load 64-bit GDT
    lgdt [gdt64.pointer]
    
    ; Jump to 64-bit code
    jmp gdt64.code:long_mode_start

; Set up identity-mapped page tables
; Maps first 2MB of physical memory
setup_page_tables:
    ; Map P4[0] -> P3
    mov eax, p3_table
    or eax, 0b11            ; Present + writable
    mov [p4_table], eax
    
    ; Map P3[0] -> P2
    mov eax, p2_table
    or eax, 0b11            ; Present + writable
    mov [p3_table], eax
    
    ; Map P2[0] -> 0MB (2MB huge page)
    mov eax, 0x0
    or eax, 0b10000011      ; Present + writable + huge page
    mov [p2_table], eax
    
    ret

; Enable paging and enter long mode
enable_paging:
    ; Load P4 table address into CR3
    mov eax, p4_table
    mov cr3, eax
    
    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5          ; Set PAE bit
    mov cr4, eax
    
    ; Enable long mode in EFER MSR
    mov ecx, 0xC0000080     ; EFER MSR
    rdmsr
    or eax, 1 << 8          ; Set LM bit
    wrmsr
    
    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31         ; Set PG bit
    mov cr0, eax
    
    ret

; 64-bit code starts here
bits 64
long_mode_start:
    ; Clear segment registers
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call kernel main with preserved multiboot info
    ; System V AMD64 ABI: first arg in RDI, second in RSI
    ; (edi and esi were preserved from 32-bit mode)
    call kernel_main
    
    ; If kernel_main returns, halt
.hang:
    cli
    hlt
    jmp .hang

; Global Descriptor Table for 64-bit mode
section .rodata
gdt64:
    dq 0                                    ; Null descriptor
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; Code segment
.pointer:
    dw $ - gdt64 - 1                        ; GDT size
    dq gdt64                                ; GDT address
```

**What's happening here?**

**32-bit setup (bits 32):**

1. **Save multiboot info**: GRUB passes magic in `EAX` and info pointer in `EBX`
2. **Set up stack**: Point ESP to our 16KB stack
3. **Set up page tables**: Identity-map first 2MB (0x0 → 0x0)
4. **Enable PAE**: Required for 64-bit paging
5. **Enable long mode**: Set LM bit in EFER MSR (Model-Specific Register)
6. **Enable paging**: Set PG bit in CR0
7. **Load 64-bit GDT**: Segment descriptors for long mode
8. **Far jump**: Jump to 64-bit code segment

**64-bit code (bits 64):**

1. **Clear segment registers**: Not used in long mode, but should be zero
2. **Call kernel**: Pass multiboot info via System V AMD64 ABI (RDI, RSI)
3. **Halt on return**: If kernel returns, halt forever

> **The Crux: Why All This Setup?**
>
> GRUB's Multiboot2 with architecture=0 boots in 32-bit protected mode. But we compiled our kernel for x86_64 (64-bit). We can't just call 64-bit code from 32-bit mode—the CPU would triple-fault! We need to enable "long mode" which requires:
>
> - **Paging enabled** with 4-level page tables (P4 → P3 → P2 → physical)
> - **PAE enabled** (Physical Address Extension)
> - **LM bit set** in the EFER MSR
> - **64-bit GDT loaded** with proper code segment
>
> Only then can we jump to 64-bit code. This is one of the trickier parts of OS development—getting the CPU into the mode you need.

## Page Tables and Long Mode

We allocate three page table levels in `.bss`:

```nasm
align 4096
p4_table:    ; Level 4 (PML4)
    resb 4096
p3_table:    ; Level 3 (PDPT)
    resb 4096  
p2_table:    ; Level 2 (PD)
    resb 4096
```

Each page table is 4KB (one page) and must be page-aligned. We use 2MB huge pages for simplicity—this lets us skip Level 1 (page table) and map 2MB chunks directly.

**Identity mapping:** We map virtual address 0x0 → physical address 0x0. Our kernel at 0x100000 (1MB) is within the first 2MB, so it's accessible.

```
Virtual Address 0x100000
    |
    v
P4[0] -> P3[0] -> P2[0] -> Physical 0x0-0x200000 (2MB huge page)
                               |
                               v
                         Physical 0x100000 (our kernel!)
```

## The Stack

We allocate a 16KB stack in `.bss`:

```nasm
align 16
stack_bottom:
    resb 16384      ; 16KB stack
stack_top:
```

The stack grows **downward** on x86-64. We point ESP/RSP to `stack_top`—when we `push`, the pointer decrements toward `stack_bottom`.

Why 16KB? It's arbitrary but safe for early boot code. Later we'll use per-CPU kernel stacks (typically 16KB-64KB each).

## Building the Boot Module

Now we need to tell CMake how to assemble these NASM files. Create `boot/CMakeLists.txt`:

```cmake
# boot/CMakeLists.txt
enable_language(ASM_NASM)

# Assemble multiboot header
add_library(multiboot OBJECT multiboot.asm)
set_target_properties(multiboot PROPERTIES
    NASM_OBJ_FORMAT elf64
)

# Assemble boot entry
add_library(boot OBJECT boot.asm)
set_target_properties(boot PROPERTIES
    NASM_OBJ_FORMAT elf64
)

# Make these available to kernel
set(BOOT_OBJECTS
    $<TARGET_OBJECTS:multiboot>
    $<TARGET_OBJECTS:boot>
    PARENT_SCOPE
)
```

**What's happening here?**

- `OBJECT` libraries compile to `.o` files but don't link yet
- `NASM_OBJ_FORMAT elf64` tells NASM to output 64-bit ELF objects
- `PARENT_SCOPE` makes `BOOT_OBJECTS` visible to the root `CMakeLists.txt`

These object files get linked into the final kernel executable. The Multiboot header goes first (remember, GRUB scans the first 32KB), followed by the boot entry code.

---

**Next: [The Linker Script →](linker-script.md)**
