# _start — Kernel Entry Points

This module produces the final `kernel-<protocol>.elf` by linking all kernel subsystems and libraries.
It defines the assembly entry point and linker script for each architecture and boot protocol.
At the end of the assembly entry point, after the stack has been setup and the CPU has been transfered to the proper mode of execution, we call the entry point of "our C world", the function `kernel_main()`.

## Supported architectures

- [x86_64](./x86_64/README.md)
- [aarch64](./aarch64/README.md)
