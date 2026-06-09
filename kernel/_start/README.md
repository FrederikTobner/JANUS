# _start — Kernel Entry Points

Assembly entry points, linker scripts, and boot-protocol request structures.
This module produces the final `kernel-<protocol>.elf` by linking all kernel
subsystems and libraries.
While aarch64 assembly code is written using the GNU assembler syntax, x86_64 assembly is written in Intel syntax for readability.

## Responsibilities

- Defines the assembly entry point for each architecture and boot protocol.
- Provides the linker script that places kernel sections at the correct addresses.
- Declares the boot-protocol request structures that the bootloader reads before
  jumping to the entry point.
- Calls the `kernel_main()` function once the stack is set up and the CPU is in the right mode.

