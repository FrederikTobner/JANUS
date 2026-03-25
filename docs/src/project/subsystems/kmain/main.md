# kernel_main

`kernel_main()` is the C entry point of the kernel, called by the architecture-specific assembly entry points after hardware initialisation is complete.

## Source Files

| File | Description |
|------|-------------|
| `src/main.c` | `kernel_main()` implementation |

## Flow

`kernel_main()` allocates a `kernel_descriptor_t` on the stack — a top-level aggregate that embeds the `boot_context_t` and will eventually hold other subsystem state. It then calls `boot_init()` to populate the boot context, initialises subsystems in the correct order, prints the kernel banner, and halts the CPU.

The current sequence is:

1. `boot_init(&kd.boot)` — populate the boot context from firmware data.
2. `kinit_serial(&kd.boot)` — bring up the serial driver (COM1 on x86_64, PL011 on aarch64).
3. `kinit_tty(&kd.boot)` — initialise the TTY (VGA text mode or framebuffer, depending on protocol and architecture).
4. Print the kernel banner.
5. `drivers_cpu_halt_forever()` — halt the CPU in an infinite loop.
