# x86_64 assembly entry point

Since the kernel supports both limine and multiboot2, there are two distinct entry points for x86_64, each requesting the appropriate boot protocol from the bootloader. 
Both entry points are used to create a dedicated ELF file, which can be packaged into a bootable ISO or loaded directly by the bootloader for testing and debugging.
The assembly code is written in Intel syntax and assembled using NASM (Netwide Assembler).
