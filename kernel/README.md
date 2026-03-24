# Kernel Root Directory

Contains all the code needed to create the JANUS kernel. The kernel is built as
a freestanding executable (`kernel.elf`) that can be booted by Limine or GRUB
(Multiboot2).

## Sub Folders

* **_start/** — Entry point module that creates `kernel.elf` by linking all
  subsystems together. Organized by architecture and boot protocol. Contains
  assembly entry points, linker scripts, and protocol-specific request
  structures.
* **kmain/** — Kernel main module containing `kernel_main()`. Responsible for
  allocating the `kernel_descriptor_t` on the stack, calling `boot_init()` to
  populate it, and initializing subsystems (serial, TTY).
* **lib/** — Kernel utility libraries providing common functionality used by
  multiple subsystems. Each library is isolated and cannot depend on other
  kernel libraries or subsystems. Currently contains `page_tables`.
* **include/janus/** — Global kernel headers defining core types, constants,
  and attributes used across the kernel (`types.h`, `attributes.h`, `config.h`).
* **subsys/** — The main kernel subsystems (boot, drivers, mm). Each subsystem
  is organized into its own directory and can contain architecture-specific code
  in an `arch/` subdirectory. Subsystems are isolated from each other — only
  `kmain` may depend on subsystems.
