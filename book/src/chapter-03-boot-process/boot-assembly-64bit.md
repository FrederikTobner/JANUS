# Understanding the 64-bit Transition

In the previous section, we stated that when boot our kernel using GRUB, we will start in 32-bit protected mode with paging disabled.
Therefor we will need to handle the transition ourself.
Some other bootloaders already handle the transition like Limine. So why are we doing this?

Because understanding the transition teaches a couple of **fundamental OS concepts** that we will need later.
These are cpu operating modes, page table structure, control registers, the relationship between paging and long mode, and the GDT structure.

GDT stands for Global Descriptor Table. It is a binary data structure that defines memory segments and their properties.
In is only relevant for to the IA-32 and x86-64 architectures.
The memory segments are descripted by seperate segment descriptors, that are defined in the GDT.
Each entry has a very complex structure, having non-contiguos memory sections for the various properties.
These properties are the base address, the segment limit, access rights, and other flags.
The memory layout of the full descriptor looks like this:

| 0x3F&nbsp;–&nbsp;0x38 | 0x37&nbsp;–&nbsp;0x34 | 0x33&nbsp;–&nbsp;0x30 | 0x2F&nbsp;–&nbsp;0x28 | 0x27&nbsp;–&nbsp;0x10 | 0x0F&nbsp;–&nbsp;0x00 (Limit) |
|:------------------|:------------------|:------------------|:------------------|:------------------|:-------------------------|
| **Base**<br>8 MSB | **Flags**  | **Limit**<br> 4 MSB | **Access Byte** | **Base**<br> 24 LSB | **Limit**<br> 16 LSB      |

[!side]  
Ever notice how the x86 segment descriptor layout feels like a memory scavenger hunt? The fields are scattered all over to place. 
The 8 most signifiant bits of the base are stored from 0x3F - 0x38, then the 24 least significant bits are stored from 0x27 to 0x10. The limit is split too, with 4 bits at 0x33-0x30 and 16 bits at 0x0F-0x00. And don't get me started on the access byte and flags in between, some of their individual bits are as mysterious as a `Sys Req` key on a keyboard.
You practically need a treasure map and and a compass to piece together a single segment descriptor! 
[/!side]

These concepts are essential for fully understanding the later chapters. 
Additionally if you would like to support Bootloaders that leave you in 32-bit mode like GRUB you need to do this manually.

Therfor we will implement what other bootloaders do behind the scenes. The transition requires:

```mermaid
flowchart TD
    step1["Save multiboot parameters"]
    step2["Set up page tables (identity mapping)"]
    step3["Enable Physical Address Extension"]
    step4["Set long mode bit in EFER MSR"]
    step5["Enable paging and activate long mode"]
    step6["Load 64-bit Global descriptor Table (GDT)"]
    step7["Jump to 64-bit code"]
    step8["Restore parameters & call kernel_main"]
    step1 --> step2
    step2 --> step3
    step3 --> step4
    step4 --> step5
    step5 --> step6
    step6 --> step7
    step7 --> step8
```

Let's implement this step-by-step. 
We'll build `kernel/boot/boot.asm` incrementally, adding each piece as we explain it.


Start with the global declarations:

```x86asm-diff
file: kernel/boot/boot.asm
replace: entire file
---
+global _start
+extern kernel_main
+
```

```x86asm-diff
file: kernel/boot/boot.asm
replace: entire file
---
global _start
extern kernel_main
+section .bss
+align 16
+stack_bottom:
+    resb 16384              
+stack_top:
```

We reserve space in the BSS (uninitialized data) section for our stack (16KiB) and page tables.

```x86asm-diff
file: kernel/boot/boot.asm
replace: entire file
---
    resb 16384              
stack_top:
+align 4096
+p4_table:
+    resb 4096
+p3_table:
+    resb 4096
+p2_table:
+    resb 4096
```

The page tables must be 4096-byte (4KiB) aligned because that's the page size the CPU expects.

Add the entry point where GRUB transfers control to us.
When GRUB loads a 64-bit kernel, it puts us in 32-bit protected mode with paging disabled.
The CPU register `EAX` contains the multiboot magic number (0x36d76289) and `EBX` contains a pointer to the multiboot info structure.
These registers are used under x86 calling conventions to pass the first two function arguments.

But the calling convention for System V AMD64 ABI expects the first two arguments in `RDI` and `RSI`, so we need to transfer the values from `EAX` to `EDI` and from `EBX` to `ESI`.
So when we call `kernel_main(uint32_t magic, void *info)`, the magic number needs to be placed in RDI and the info pointer in RSI.

> **New to the System V AMD64 ABI?**
>
> ABI stands for Application Binary Interface. 
> It defines the rules for how functions are called at the assembly level and ensures that code that follows the ABI conventions can also be used under different architectures that implement the same ABI.
> The System V AMD64 ABI, that is used by Linux, BSD, and most Unix-like systems, specifies the first six integer or pointer arguments are passed in registers, and the return value is also passed in a register. 
> Additionally it defines that the stack must be 16-byte aligned before a call instruction. Lastly, it specifies which registers are caller-saved and which are callee-saved.

```x86asm-diff
file: kernel/boot/boot.asm
after: resb 4096
---
 p2_table:
     resb 4096
+
+section .text
+bits 32                     
+
+_start:
+    mov edi, eax            
+    mov esi, ebx           
```

Next we setup the stack pointer register `ESP` to point to the top of our stack. 
Under x64 the stack is commonly called `RSP`, but in 32-bit mode it is still `ESP`, even though they are referencing the same physical register.
Then we call a subroutine to setup the page tables and enable long mode.

Additionally we need to load the global descriptor table and perform a far jump to switch to the 64-bit code segment.

```x86asm-diff
file: kernel/boot/boot.asm
after: resb 4096
---
_start:
    mov edi, eax          
    mov esi, ebx         
+    
+    mov esp, stack_top
+    
+    call setup_page_tables
+    call enable_paging
+    
+    lgdt [gdt64.pointer]
+    
+    jmp gdt64.code:long_mode_start
```

Now add the function that creates our page tables:

```x86asm-diff
file: kernel/boot/boot.asm
after: jmp gdt64.code:long_mode_start
---
     jmp gdt64.code:long_mode_start
+
+setup_page_tables:
+    mov eax, p3_table
+    or eax, 0b11            
+    mov [p4_table], eax
+    
+    mov eax, p2_table
+    or eax, 0b11            
+    mov [p3_table], eax
+    
+    mov eax, 0x0
+    or eax, 0b10000011      
+    mov [p2_table], eax
+    
+    ret
```

We create a simple identity mapping: virtual address 0x0 → physical address 0x0 for the first 2MB. The page entry flags are:

- Bit 0 (Present): Page is present in memory
- Bit 1 (Writable): Page can be written to
- Bit 7 (Huge page): This is a 2MB page, not a 4KB page

The flag value `0b11` equals \\(2^0 + 2^1 = 3\\) (present + writable), and `0b10000011` equals \\(2^0 + 2^1 + 2^7 = 131\\) (present + writable + huge page).

We're using a 2MB **huge page** which skips the P1 (page table) level entirely. This maps the entire first 2MB in one entry instead of 512 individual 4KB pages. A standard 4KB page would require \\(512\\) entries (since \\(2\\text{MB} = 2^{21}\\) bytes and \\(4\\text{KB} = 2^{12}\\) bytes, so \\(2^{21} / 2^{12} = 2^9 = 512\\) pages).

Add this subroutine to enable paging.

```x86asm-diff
file: kernel/boot/boot.asm
after: setup_page_tables ret
---
     ret
+
+enable_paging:
+    mov eax, p4_table
+    mov cr3, eax
+    
+    ret
```

Fist we load the P4 table address into control register CR3, which tells the CPU where our page tables are located in memory.

Next we enable the Physical Address Extension (PAE) by setting bit 5 in control register CR4. PAE allows the CPU to access more than 4GB of physical memory and is a prerequisite for entering long mode.
```x86asm-diff
file: kernel/boot/boot.asm
after: mov cr3, eax 
---
enable_paging:
    mov eax, p4_table
    mov cr3, eax
    
+    mov eax, cr4
+    or eax, 1 << 5          
+    mov cr4, eax
+
    ret
```

After that we set the Long Mode Enable bit in the Extended Feature Enable Register (EFER) Model-Specific Register (MSR) using the `rdmsr` and `wrmsr` instructions. This tells the CPU that we want to enter 64-bit mode.


```x86asm-diff
file: kernel/boot/boot.asm
after: mov cr4, eax 
---
    mov eax, cr4
    or eax, 1 << 5          
    mov cr4, eax
    
+    mov ecx, 0xC0000080     
+    rdmsr
+    or eax, 1 << 8          
+    wrmsr
+    
    ret
```

Finally, we enable paging by setting the Paging (PG) bit in control register CR0. Once paging is enabled and long mode is set, the CPU transitions to 64-bit mode.

```x86asm-diff
file: kernel/boot/boot.asm
after: wrmsr 
---
    mov ecx, 0xC0000080     
    rdmsr
    or eax, 1 << 8          
    wrmsr
    
+    mov eax, cr0
+    or eax, 1 << 31          
+    mov cr0, eax
    
    ret
```

> **Aside: Page Table Naming**
>
> The page table names used by Intel and AMD are quite confusing, because use different terminology:
>
> - **P4/PML4** = Page Map Level 4 (top level)
> - **P3/PDPT** = Page Directory Pointer Table
> - **P2/PD** = Page Directory
> - **P1/PT** = Page Table (we skip this by using huge pages)
>
> We use the shorter P4/P3/P2 names for simplicity.

We're finally in long mode! Add the 64-bit entry point that calls our kernel:

```x86asm-diff
file: kernel/boot/boot.asm
after: enable_paging ret
---
     ret
+
+bits 64
+long_mode_start:
+    xor ax, ax
+    mov ss, ax
+    mov ds, ax
+    mov es, ax
+    mov fs, ax
+    mov gs, ax
+    
+    call kernel_main
+    
+.hang:
+    cli
+    hlt
+    jmp .hang
```

```x86asm-diff
file: kernel/boot/boot.asm
after: enable_paging ret
---
    call kernel_main
    
+.hang:
+    cli
+    hlt
+    jmp .hang
```


After `kernel_main` returns, we enter an infinite loop that halts the CPU.
You might be wondering why we need this, because our kernel should never return from `kernel_main`.
This is only a defensive measure to ensure the CPU doesn't execute random instructions if `kernel_main` were to return.

[!side]
When working on a kernel, it is crucial to be very defensive, you could even say almost paranoid. 
Like what happens if my kernel, has a broken BIOS, or a cosmic ray flips a bit in your boot info? 
Always expect the unexpected!
[/side]

Now we can actually start writing 64-bit code after we specified the target processor mode using the bits directive.
In long mode, segment registers aren't used for addressing (flat memory model), but we zero them out for cleanliness. The EDI and ESI registers we saved in Step 2 are now RDI and RSI, perfectly positioned as the first two function arguments per the System V AMD64 calling convention. After we call `kernel_main`, we use a 'hlt' instruction in an infinite loop to halt the CPU when the kernel returns.

Finally, add the GDT that defines our 64-bit code segment:

```x86asm-diff
file: boot/boot.asm
after: .hang loop
---
     jmp .hang
+
+section .rodata
+gdt64:
+    dq 0                                    
+.code: equ $ - gdt64
+    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) 
+.pointer:
+    dw $ - gdt64 - 1                        
+    dq gdt64                                
```

The code segment descriptor sets bits for: executable (bit 43), code/data segment (bit 44), present (bit 47), and 64-bit mode (bit 53). The value \\((1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)\\) creates a 64-bit value with these specific bits set. 


```mermaid
graph LR
    A["<b>32-bit Mode</b><br/>(GRUB gives us)<br/><br/>32-bit regs<br/>ESP stack<br/>EAX = magic<br/>EBX = info"]
    B["<b>Transition</b><br/>(Our boot.asm does)<br/><br/>1. Page tables<br/>2. Enable PAE<br/>3. Set EFER.LM<br/>4. Enable paging<br/>5. Load GDT64<br/>6. Far jump"]
    C["<b>64-bit Mode</b><br/>(kernel_main runs)<br/><br/>64-bit regs<br/>RSP stack<br/>RDI = magic<br/>RSI = info"]
    
    A -->|"Protected Mode"| B
    B -->|"The Scary Part<br/>(Don't mess up!)"| C
    C -.->|"Long Mode Enabled!"| C
```

Now lets validate that out boot assembly transition to 64-bit mode works properly.

> TODO: Test registers and show variables

Now rebuild and test:

```bash
ninja -C build
ninja -C build run
```

---

**Next:** [Summary](./summary.md)
