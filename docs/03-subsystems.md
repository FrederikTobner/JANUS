# Subsystems

## boot

Protocol-agnostic boot context and protocol-specific initialization.
Defines `boot_context_t` and provides `boot_init()`, which is linked from the correct protocol library (`boot_limine` or `boot_multiboot2`) per build.
See [boot/README.md](../kernel/subsys/boot/README.md).

## drivers

Hardware-facing device drivers: serial (UART), TTY (text terminal and framebuffer), and CPU control (`drivers_cpu_halt_forever`).
See [drivers/README.md](../kernel/subsys/drivers/README.md).

## interrupts

Interrupt subsystem. Currently only supports x86_64.

## mm

Memory management subsystem.
Contains currently only a bitmap-based physical memory manager.
See [mm/README.md](../kernel/subsys/mm/README.md).

## kmain

`kmain` is the composition root of the kernel.
It is the only module permitted to depend on subsystems, and it is responsible for sequencing the initialisation of everything above the boot entry point.

More information can be found in its [readme](../kernel/kmain/README.md)
