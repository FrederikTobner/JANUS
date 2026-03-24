# Kernel Main

`kmain` is the composition root of the kernel. It is the only module permitted to depend on subsystems, and it owns the initialisation sequence that brings the kernel from raw boot data to a running state.

## Responsibilities

`kernel_main()` allocates a `kernel_descriptor_t` on the stack — a top-level aggregate that embeds the `boot_context_t` and will eventually hold other subsystem state. It then calls `boot_init()` to populate the boot context, initialises subsystems in the correct order, prints the kernel banner, and halts the CPU.

The current init sequence is:

1. `boot_init(&kd.boot)` — populate the boot context from firmware data.
2. `kinit_serial(&kd.boot)` — bring up the serial driver (COM1 on x86_64, PL011 on aarch64).
3. `kinit_tty(&kd.boot)` — initialise the TTY (VGA text mode or framebuffer, depending on protocol and architecture).
4. Print the kernel banner.
5. `drivers_cpu_halt_forever()` — halt the CPU in an infinite loop.

## The Kernel Descriptor

`kernel_descriptor_t` is defined in `internal/kmain/kernel_descriptor.h` and is private to `kmain`. It currently contains only a `boot_context_t`, but is designed to grow as new subsystems are added. The descriptor is always stack-allocated and its address is never stored globally — subsystem init functions receive pointers to the specific fields they need.

## Layering

`kmain` sits between the assembly entry points (the `_start` layer) and the subsystem layer. It is the sole module that links against subsystems, enforcing a star topology where all inter-subsystem data flow is mediated through `kmain` rather than through direct subsystem-to-subsystem calls.

```text
_start  →  kernel_main (kmain)  →  boot, drivers (subsys)  →  page_tables (lib)
```
