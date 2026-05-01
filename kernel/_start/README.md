# _start — Kernel Entry Points

Assembly entry points, linker scripts, and boot-protocol request structures.
This module produces the final `kernel-<protocol>.elf` by linking all kernel
subsystems and libraries.

## Responsibilities

- Define the assembly entry point for each architecture and boot protocol.
- Provide the linker script that places kernel sections at the correct addresses.
- Declare boot-protocol request structures that the bootloader reads before
  jumping to the entry point.
- Call `kernel_main()` once the stack is set up and the CPU is in the right mode.

## Layout

```
_start/
├── common/
│   └── limine_requests.c   # Limine request symbols (shared across architectures)
├── x86_64/
│   ├── limine/             # x86_64 Limine entry point + linker script
│   └── multiboot2/         # x86_64 Multiboot2 entry point + linker script
└── aarch64/
    └── limine/             # AArch64 Limine entry point + linker script
```

## What the entry point does

1. Sets up an initial stack.
2. On x86_64 Multiboot2: calls `multiboot2_stash_bootinfo` to save the magic
   value and info pointer from registers before the ABI clobbers them.
3. Falls through to `kernel_main()` (declared `__noreturn`).

## Build

`janus_add_kernel()` in `cmake/kernel/Executable.cmake` links the object files
from the selected boot protocol with all registered subsystem and library
targets to produce the final ELF.
