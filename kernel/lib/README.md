# lib — Kernel Libraries

Freestanding utility libraries shared across subsystems. Libraries may depend
on `janus_asm`, but never on other libraries, any core services or any subsystems.

## Libraries

### display

Contains a framebuffer implementation for drawing pixels, rectangles, and text to a buffer.

### fmt

Formatted output to an arbitrary sink. 
Provides `printf`-style formatting without depending on `libc`. 
Consumers first supply a `fmt_putc_fn` callback, which is called for each character of the formatted output. 

### page\_tables *(aarch64 only)*

MMU library for creating and managing 4-level page tables on AArch64.
Maps physical addresses into the kernel virtual address space. 

