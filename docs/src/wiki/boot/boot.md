# Boot

This section covers the chain of events between pressing the power button and executing the first line of kernel code. No prior OS knowledge is assumed.

## Articles

Read in order — each article assumes the previous ones.

1. **[Firmware](firmware.md)** — What the machine does before any software is loaded: BIOS vs UEFI, the POST, and what the firmware provides to the bootloader.
2. **[Bootloaders](bootloaders.md)** — Why a kernel cannot boot directly from firmware, what a bootloader does, and how boot protocols define the contract between bootloader and kernel.
3. **[Freestanding Environments](freestanding.md)** — What it means for a C program to run without an operating system or standard library, and what the compiler and linker require for a freestanding build.
4. **[Protocols](protocols/protocols.md)** — The two protocols JANUS supports, with a comparison of their approaches:
   - **[Limine](protocols/limine.md)** — request/response model, higher-half entry, x86_64 and aarch64.
   - **[Multiboot2](protocols/multiboot2.md)** — tag-header model, 32-bit protected mode entry, x86 only.
