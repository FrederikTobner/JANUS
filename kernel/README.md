# Kernel root directory

Contains all the code needed to create the JANUS kernel. The kernel is built as a freestanding executable (`kernel.elf`) that can be booted by a bootloader like GRUB.

## Sub Folders

* **_start/** - Entry point module that creates `kernel.elf` by linking all subsystems together. Organized by architecture and boot protocol.
* **kmain/** - Kernel main module that contains the `kmain` function and is responsible for initializing the kernel subsystems and starting the main loop.
* **lib/** - Kernel utility libraries that provide common functionality used by multiple subsystems. Each library is isolated and cannot depend on other kernel libraries or modules.
* **include/janus/** - Global kernel headers that define core types, constants, and attributes used across the kernel.
* **subsystems/** - The main kernel subsystems (e.g., memory management, process management, device drivers). Each subsystem is organized into its own directory and can contain architecture-specific code in an `arch/` subdirectory.

