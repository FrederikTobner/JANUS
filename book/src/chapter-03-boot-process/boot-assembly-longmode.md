# Transitioning to Long Mode

In the previous section, we learned the hard way that you can't call 64-bit code from 32-bit mode. Now let's fix our boot code by transitioning the CPU to 64-bit long mode before calling our kernel.

## What We Need to Do

GRUB boots us in **32-bit protected mode**, but our kernel needs **64-bit long mode**. The transition requires:

[!side]
Unlike 32-bit mode, long mode *requires* paging to be enabled. No paging = no 64-bit mode.
[/!side]

1. Set up page tables (long mode requires paging)
2. Enable PAE (Physical Address Extension)
3. Set the long mode bit in EFER MSR
4. Enable paging
5. Load a 64-bit GDT
6. Perform a far jump to 64-bit code

Let's implement each step.

## The Complete Boot Assembly

Replace `boot/boot.asm` with this corrected version:

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

[!side]
PAE extends 32-bit addressing to 36 bits (64GB). Required stepping stone to 64-bit mode.
[/!side]
    
    ; Enable long mode in EFER MSR
    mov ecx, 0xC0000080     ; EFER MSR
    rdmsr
    or eax, 1 << 8          ; Set LM bit
    wrmsr

[!side]
EFER = Extended Feature Enable Register. MSRs are special CPU registers accessed with `rdmsr`/`wrmsr`.
[/!side]
    
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

## Understanding the Transition

### CPU Mode Transition Journey

```
The 32-bit to 64-bit Transition:

  32-bit Mode              Transition                64-bit Mode
  (GRUB gives us)      (Our boot.asm does)        (kernel_main runs)
┌──────────────┐      ┌──────────────────┐       ┌──────────────┐
│ 32-bit regs  │      │ 1. Page tables   │       │ 64-bit regs  │
│ ESP stack    │ ───▶ │ 2. Enable PAE    │ ───▶  │ RSP stack    │
│ EAX = magic  │      │ 3. Set EFER.LM   │       │ RDI = magic  │
│ EBX = info   │      │ 4. Enable paging │       │ RSI = info   │
│              │      │ 5. Load GDT64    │       │              │
│              │      │ 6. Far jump      │       │              │
└──────────────┘      └──────────────────┘       └──────────────┘
    Protected              The Scary Part            Long Mode
     Mode                 (Don't mess up!)            Enabled!
```

> **TODO: Hand-drawn illustration idea**
> Draw the CPU as a character going through a transformation sequence like a video game power-up. Panel 1: "32-bit CPU" looking small and limited. Panel 2: Eating a "PAE mushroom" and "EFER crystal". Panel 3: Growing bigger with sparkles, now "64-bit CPU" with flexed muscles and a cape, saying "I can address SO MUCH MEMORY now!"

## Step-by-Step Breakdown

### Step 1: Save Multiboot Parameters

```nasm
mov edi, eax            ; Save magic
mov esi, ebx            ; Save multiboot info pointer
```

GRUB passes parameters in EAX and EBX. We save them to EDI and ESI because:

- These registers are preserved across the mode switch
- In 64-bit mode, EDI becomes RDI and ESI becomes RSI
- The System V AMD64 ABI expects function arguments in RDI and RSI

### Step 2: Set Up Page Tables

```nasm
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
```

We create a simple identity mapping: virtual address 0x0 → physical address 0x0 for the first 2MB.

**Page entry flags:**

- Bit 0 (Present): Page is present in memory
- Bit 1 (Writable): Page can be written to
- Bit 7 (Huge page): This is a 2MB page, not a 4KB page

The flag value `0b11` equals \\(2^0 + 2^1 = 3\\) (present + writable), and `0b10000011` equals \\(2^0 + 2^1 + 2^7 = 131\\) (present + writable + huge page).

### 4-Level Page Table Structure

```
Virtual Address Translation (Simplified):

 Virtual Address                     Physical Address
┌────────────────┐                  ┌────────────────┐
│  0x00101234    │                  │  0x00101234    │
└────────┬───────┘                  └────────▲───────┘
         │                                   │
         ▼                                   │
    ┌─────────┐                              │
    │  CR3    │──▶ P4 Table (PML4)           │
    └─────────┘      │                       │
                     ├─▶ Entry[0]            │
                     │      │                │
                     ▼      ▼                │
                          P3 Table (PDPT)    │
                            │                │
                            ├─▶ Entry[0]     │
                            │      │         │
                            ▼      ▼         │
                                 P2 Table (PD)
                                   │
                                   ├─▶ Entry[0] (2MB huge page)
                                   │      │
                                   ▼      └──────────┘
                                         0x00000000
                                         (Identity mapped!)
```

We're using a 2MB **huge page** which skips the P1 (page table) level entirely. This maps the entire first 2MB in one entry instead of 512 individual 4KB pages.

**Math:** A standard 4KB page would require \\(512\\) entries

(since \\(2\\text{MB} = 2^{21}\\) bytes and \\(4\\text{KB} = 2^{12}\\) bytes, so \\(2^{21} / 2^{12} = 2^9 = 512\\) pages).

### Step 3: Enable PAE

```nasm
mov eax, cr4
or eax, 1 << 5          ; Set PAE bit (bit 5)
mov cr4, eax
```

**PAE (Physical Address Extension):** Originally added to 32-bit x86 to access more than 4GB of RAM, PAE extends physical addresses from 32 bits to 36 bits. In 64-bit mode, PAE is *required*—you can't enable long mode without it.

**Math:** Without PAE, 32-bit addressing allows \\(2^{32} = 4\\text{GB}\\) of physical memory. With PAE, 36-bit addressing allows \\(2^{36} = 64\\text{GB}\\) of physical memory.

### Step 4: Enable Long Mode

```nasm
mov ecx, 0xC0000080     ; EFER MSR number
rdmsr                   ; Read MSR into EDX:EAX
or eax, 1 << 8          ; Set LM bit (bit 8)
wrmsr                   ; Write back to MSR
```

**EFER (Extended Feature Enable Register):** A Model-Specific Register (MSR) that controls advanced CPU features. Bit 8 (LME - Long Mode Enable) tells the CPU we want 64-bit mode.

**MSR (Model-Specific Register):** Special registers accessed via `rdmsr` (read) and `wrmsr` (write) instructions, not like normal registers. You specify which MSR using ECX.

### Step 5: Enable Paging

```nasm
mov eax, cr0
or eax, 1 << 31         ; Set PG bit (bit 31)
mov cr0, eax
```

**CR0:** Control Register 0 contains system control flags. Setting bit 31 (PG) enables paging. Once paging is enabled *and* LME is set in EFER, the CPU enters long mode!

### Step 6: Load 64-bit GDT

```nasm
lgdt [gdt64.pointer]
```

**GDT (Global Descriptor Table):** Defines memory segments. In 64-bit long mode, segmentation is mostly disabled (flat memory model), but we still need a GDT with at least a code segment to tell the CPU we're in 64-bit mode.

Our GDT has two entries:

```nasm
gdt64:
    dq 0                                    ; Null descriptor (required)
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; Code segment (64-bit)
.pointer:
    dw $ - gdt64 - 1                        ; GDT size
    dq gdt64                                ; GDT address
```

The code segment descriptor sets:

- Bit 43: Executable
- Bit 44: Code/Data segment
- Bit 47: Present
- Bit 53: 64-bit mode

The value
\\((1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)\\)

creates a 64-bit value with these specific bits set:

\\(2^{43} + 2^{44} + 2^{47} + 2^{53}\\).

### Step 7: Far Jump to 64-bit Code

```nasm
jmp gdt64.code:long_mode_start
```

A **far jump** jumps to a different code segment. This loads the 64-bit code segment selector into CS and jumps to `long_mode_start`. At this point, we're officially in 64-bit long mode!

### Step 8: 64-bit Code Execution

```nasm
bits 64
long_mode_start:
    ; Clear segment registers
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call kernel main
    call kernel_main
```

In long mode, segment registers aren't used for addressing (flat memory model), but we zero them out for cleanliness. Then we call `kernel_main` with our preserved Multiboot parameters (now in RDI and RSI).

## Key Concepts Explained

> **Aside: System V AMD64 ABI**
>
> ABI stands for Application Binary Interface—the rules for how functions are called at the assembly level. The System V AMD64 ABI (used by Linux, BSD, and most Unix-like systems) specifies:
>
> - **First 6 integer arguments** go in registers: RDI, RSI, RDX, RCX, R8, R9
> - **Return value** goes in RAX
> - **Stack must be 16-byte aligned** before a call instruction
> - **Certain registers are callee-saved** (RBX, RBP, R12-R15)
>
> So when we call `kernel_main(uint32_t magic, void *info)`, the magic number goes in RDI and the info pointer goes in RSI.

> **Aside: Page Table Naming**
>
> The names are confusing because Intel and AMD use different terminology:
>
> - **P4/PML4** = Page Map Level 4 (top level)
> - **P3/PDPT** = Page Directory Pointer Table
> - **P2/PD** = Page Directory
> - **P1/PT** = Page Table (we skip this by using huge pages)
>
> We use the shorter P4/P3/P2 names for simplicity.

## Testing the Fixed Boot Code

Now rebuild and test:

```bash
ninja -C build
ninja -C build run
```

**Result:** QEMU opens with a blank screen and doesn't crash! The kernel is running. Press Ctrl+C to exit.

We can verify it's working with LLDB (covered in the "Booting Up" chapter).

## What We've Accomplished

Our boot assembly now:

- Starts in 32-bit protected mode (as GRUB gives us)
- Sets up identity-mapped page tables for the first 2MB
- Enables PAE, long mode, and paging
- Loads a 64-bit GDT
- Transitions to 64-bit long mode
- Calls our 64-bit kernel successfully

No more triple faults!

---

**Next: [The Linker Script](linker-script.md)**
