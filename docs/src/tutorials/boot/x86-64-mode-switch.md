# Transitioning to 64-bit Long Mode

This tutorial walks through the x86_64 mode switch — the sequence of steps a Multiboot2 kernel must perform to transition from the 32-bit protected mode that GRUB establishes to the 64-bit long mode that the kernel runs in. The tutorial is self-contained: it assumes a Multiboot2-compliant bootloader has loaded the kernel and transferred control to a `_start` symbol in 32-bit mode.

## Prerequisites

The reader should be comfortable with basic x86 assembly (NASM syntax) and understand what registers, the stack, and memory-mapped structures are. Familiarity with the Multiboot2 entry state is helpful — see [Multiboot2](../../wiki/boot/multiboot2.md) for the full specification. The concepts behind paging are covered in [Virtual Memory](../../wiki/memory/virtual-memory.md).

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

```x86asm-diff
file: boot.asm
after: stack reservation
---
 stack_bottom:
     resb 16384          ; 16 KiB stack
 stack_top:

+align 4096
+p4_table:
+    resb 4096           ; PML4 (Page Map Level 4)
+p3_table:
+    resb 4096           ; PDPT (Page Directory Pointer Table)
+p2_table:
+    resb 4096           ; PD   (Page Directory)
```

Each table must be 4 KiB-aligned because the CPU uses the lower 12 bits of each entry for flags. The `align 4096` directive ensures this.

Now wire the tables together and create the mapping. The subroutine `setup_page_tables` begins by linking the three levels. Each entry contains the physical address of the next-level table OR'd with flag bits.

Point the first PML4 entry at the PDPT, and the first PDPT entry at the Page Directory. The flag `0b11` (bits 0 and 1) marks each entry as present and writable:

```x86asm-diff
file: boot.asm
after: _start
---
+setup_page_tables:
+    mov eax, p3_table
+    or eax, 0b11
+    mov [p4_table], eax
+
+    mov eax, p2_table
+    or eax, 0b11
+    mov [p3_table], eax
```

The first Page Directory entry maps physical address `0x0` as a 2 MiB huge page. Bit 7 tells the CPU this entry maps a region directly rather than pointing to another table level, eliminating the need for a PT:

```x86asm-diff
file: boot.asm
after: PML4 and PDPT entries
---
+    mov eax, 0x0
+    or eax, 0b10000011
+    mov [p2_table], eax
+
+    ret
```

## Step 3: Enable Paging and Long Mode

The mode switch itself touches three control registers and one model-specific register. Each step has strict prerequisites, so the order matters.

First, load the physical address of the PML4 table into CR3 so the CPU knows where the page tables live:

```x86asm-diff
file: boot.asm
after: setup_page_tables
---
+enable_paging:
+    mov eax, p4_table
+    mov cr3, eax
```

Enable Physical Address Extension (PAE) by setting bit 5 of CR4. PAE is a prerequisite for long mode — without it, the CPU cannot use 4-level page tables:

```x86asm-diff
file: boot.asm
after: load CR3
---
+    mov eax, cr4
+    or eax, 1 << 5
+    mov cr4, eax
```

Set the Long Mode Enable (LME) bit in the EFER model-specific register. The EFER is accessed via `rdmsr`/`wrmsr` at address `0xC0000080`; bit 8 is LME:

```x86asm-diff
file: boot.asm
after: enable PAE
---
+    mov ecx, 0xC0000080
+    rdmsr
+    or eax, 1 << 8
+    wrmsr
```

Finally, enable paging by setting bit 31 of CR0. Once this bit goes high, the CPU activates the page tables loaded in CR3:

```x86asm-diff
file: boot.asm
after: set EFER.LME
---
+    mov eax, cr0
+    or eax, 1 << 31
+    mov cr0, eax
+
+    ret
```

After `CR0.PG` is set, the CPU is in *compatibility mode* — a transitional state where it uses 64-bit page tables but still executes 32-bit code. The final step is a far jump through a 64-bit code segment to fully enter long mode.

## Step 4: Load a 64-bit GDT and Jump

Define a minimal Global Descriptor Table in the read-only data section. The first entry is always the null descriptor (required by the CPU):

```x86asm-diff
file: boot.asm
after: enable_paging
---
+section .rodata
+gdt64:
+    dq 0
```

The code segment descriptor sets four bits: executable (43), code/data (44), present (47), and 64-bit (53). In long mode, segmentation is essentially flat — the base and limit fields are ignored, but the GDT is still required for the mode switch:

```x86asm-diff
file: boot.asm
after: null descriptor
---
+.code: equ $ - gdt64
+    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
```

The GDT pointer is a 10-byte structure that `lgdt` reads: a 2-byte limit followed by an 8-byte base address:

```x86asm-diff
file: boot.asm
after: code segment descriptor
---
+.pointer:
+    dw $ - gdt64 - 1
+    dq gdt64
```

Back in `_start`, add the calls and the far jump:

```x86asm-diff
file: boot.asm
replace: _start
---
 _start:
     mov edi, eax
     mov esi, ebx
     mov esp, stack_top

+    call setup_page_tables
+    call enable_paging
+
+    lgdt [gdt64.pointer]
+    jmp gdt64.code:long_mode_start
```

The `jmp gdt64.code:long_mode_start` is a far jump that loads the code segment selector from the GDT and switches the CPU to 64-bit execution.

## Step 5: Enter 64-bit Code

Switch NASM to 64-bit mode and define the target of the far jump. The first thing to do in long mode is zero out the segment registers — they are unused in 64-bit mode but should be in a clean state:

```x86asm-diff
file: boot.asm
after: GDT
---
+bits 64
+long_mode_start:
+    xor ax, ax
+    mov ss, ax
+    mov ds, ax
+    mov es, ax
+    mov fs, ax
+    mov gs, ax
```

With the segment registers clean, call into C. `RDI` and `RSI` already hold the Multiboot2 magic and info pointer (the 32-bit values saved in step 1 are zero-extended), so the System V AMD64 calling convention is satisfied:

```x86asm-diff
file: boot.asm
after: zero segment registers
---
+    call kernel_main
```

The `.hang` loop after `kernel_main` is defensive. If the kernel ever returns, disable interrupts and halt the CPU in a tight loop rather than executing whatever happens to follow in memory:

```x86asm-diff
file: boot.asm
after: call kernel_main
---
+.hang:
+    cli
+    hlt
+    jmp .hang
```

## Key Takeaways

- **The mode switch is a strict sequence of register manipulations.** CR3 (page table base), CR4.PAE, EFER.LME, CR0.PG — each step has prerequisites that must be satisfied before it.
- **Page tables must exist before paging is enabled.** There is no way to "set them up later" — the CPU reads CR3 the instant CR0.PG goes high.
- **The far jump is what switches the instruction decoder.** Setting EFER.LME and CR0.PG puts the CPU in compatibility mode (64-bit tables, 32-bit execution). Only the far jump through a 64-bit GDT code segment completes the transition.
- **Identity mapping the first 2 MiB is the minimum viable setup.** It keeps the kernel, stack, and boot info accessible at the same addresses. A production kernel would later establish a proper higher-half mapping, a larger stack, and a complete GDT.
