# Transitioning to 64-bit Long Mode

This tutorial walks through the x86_64 mode switch — the sequence of steps a Multiboot2 kernel must perform to transition from the 32-bit protected mode that GRUB establishes to the 64-bit long mode that the kernel runs in. The tutorial is self-contained: it assumes a Multiboot2-compliant bootloader has loaded the kernel and transferred control to a `_start` symbol in 32-bit mode.

## Prerequisites

The reader should be comfortable with basic x86 assembly (NASM syntax) and understand what registers, the stack, and memory-mapped structures are. Familiarity with the Multiboot2 entry state is helpful — see [Multiboot2](../concepts/boot/multiboot2.md) for the full specification. The concepts behind paging are covered in [Virtual Memory](../concepts/memory/virtual-memory.md).

## Starting Point

When GRUB enters `_start`, the machine is in this state:

- 32-bit protected mode, paging disabled
- `EAX` = `0x36D76289` (Multiboot2 bootloader magic)
- `EBX` = pointer to the boot information structure
- No usable stack
- Interrupts disabled

Our goal is to reach 64-bit long mode with a stack, a minimal identity mapping, and the Multiboot2 parameters preserved so we can pass them to C code.

## Step 1: Preserve Parameters and Set Up a Stack

The first thing the entry code does is save the two Multiboot2 values into registers that the System V AMD64 ABI uses for function arguments. When we later call `kernel_main(uint32_t magic, void *info)`, the values will already be in the right place:

```x86asm
global _start
extern kernel_main

section .bss
align 16
stack_bottom:
    resb 16384          ; 16 KiB stack
stack_top:

section .text
bits 32

_start:
    mov edi, eax        ; magic → EDI (will become RDI in 64-bit)
    mov esi, ebx        ; info  → ESI (will become RSI in 64-bit)
    mov esp, stack_top   ; set up the stack
```

The `bits 32` directive tells NASM to emit 32-bit instructions. We will switch to `bits 64` after the mode transition.

## Step 2: Build Page Tables

Long mode requires paging. We set up the minimum viable page table structure: a single 2 MiB identity mapping using a huge page. This keeps the first 2 MiB of physical memory accessible at the same virtual address — enough to contain the kernel, the stack, and the boot information structure.

Reserve space for three page table levels in the BSS section (add this alongside the stack):

```x86asm
align 4096
p4_table:
    resb 4096           ; PML4 (Page Map Level 4)
p3_table:
    resb 4096           ; PDPT (Page Directory Pointer Table)
p2_table:
    resb 4096           ; PD   (Page Directory)
```

Each table must be 4 KiB-aligned because the CPU uses the lower 12 bits of each entry for flags. The `align 4096` directive ensures this.

Now wire the tables together and create the mapping. Add this subroutine after `_start`:

```x86asm
setup_page_tables:
    ; P4[0] → P3
    mov eax, p3_table
    or eax, 0b11           ; present + writable
    mov [p4_table], eax

    ; P3[0] → P2
    mov eax, p2_table
    or eax, 0b11           ; present + writable
    mov [p3_table], eax

    ; P2[0] → 0x0 as a 2 MiB huge page
    mov eax, 0x0
    or eax, 0b10000011     ; present + writable + huge page (bit 7)
    mov [p2_table], eax

    ret
```

The flag `0b11` (bits 0 and 1) marks the entry as present and writable. The flag `0b10000011` adds bit 7, which tells the CPU that this Page Directory entry maps a 2 MiB region directly rather than pointing to another table level. This eliminates the need for a fourth-level page table (PT) for this mapping.

## Step 3: Enable Paging and Long Mode

The mode switch itself touches three control registers and one model-specific register. Add this subroutine:

```x86asm
enable_paging:
    ; Load P4 table address into CR3
    mov eax, p4_table
    mov cr3, eax

    ; Enable PAE (Physical Address Extension) — CR4 bit 5
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Set the Long Mode Enable bit in the EFER MSR (0xC0000080, bit 8)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging — CR0 bit 31
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret
```

After `CR0.PG` is set, the CPU is in *compatibility mode* — a transitional state where it uses 64-bit page tables but still executes 32-bit code. The final step is a far jump through a 64-bit code segment to fully enter long mode.

## Step 4: Load a 64-bit GDT and Jump

Define a minimal Global Descriptor Table in the read-only data section:

```x86asm
section .rodata
gdt64:
    dq 0                                        ; null descriptor
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)   ; code segment
.pointer:
    dw $ - gdt64 - 1                            ; limit
    dq gdt64                                    ; base
```

The code segment descriptor sets four bits: executable (43), code/data (44), present (47), and 64-bit (53). In long mode, segmentation is essentially flat — the base and limit fields are ignored, but the GDT is still required for the mode switch.

Back in `_start`, add the calls and the far jump:

```x86asm
_start:
    mov edi, eax
    mov esi, ebx
    mov esp, stack_top

    call setup_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start
```

The `jmp gdt64.code:long_mode_start` is a far jump that loads the code segment selector from the GDT and switches the CPU to 64-bit execution.

## Step 5: Enter 64-bit Code

```x86asm
bits 64
long_mode_start:
    ; Zero segment registers (not used in long mode, but clean state)
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
```

At this point `RDI` and `RSI` contain the Multiboot2 magic and info pointer (the 32-bit values we saved in step 1 are zero-extended). The `call kernel_main` follows the System V AMD64 ABI — the first two integer arguments are passed in `RDI` and `RSI`.

The `.hang` loop after `kernel_main` is defensive. If the kernel ever returns, the CPU halts rather than executing whatever happens to follow in memory.

## Key Takeaways

- **The mode switch is a strict sequence of register manipulations.** CR3 (page table base), CR4.PAE, EFER.LME, CR0.PG — each step has prerequisites that must be satisfied before it.
- **Page tables must exist before paging is enabled.** There is no way to "set them up later" — the CPU reads CR3 the instant CR0.PG goes high.
- **The far jump is what switches the instruction decoder.** Setting EFER.LME and CR0.PG puts the CPU in compatibility mode (64-bit tables, 32-bit execution). Only the far jump through a 64-bit GDT code segment completes the transition.
- **Identity mapping the first 2 MiB is the minimum viable setup.** It keeps the kernel, stack, and boot info accessible at the same addresses. A production kernel would later establish a proper higher-half mapping, a larger stack, and a complete GDT.
