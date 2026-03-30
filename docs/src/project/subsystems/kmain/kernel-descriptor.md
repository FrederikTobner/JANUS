# Kernel Descriptor

`kernel_descriptor_t` is the top-level state aggregate that collects all subsystem state into a single stack-allocated value.

## Source Files

| File | Description |
|------|-------------|
| `internal/kmain/kernel_descriptor.h` | Structure definition (private to `kmain`) |

## Design

The descriptor currently contains only a `boot_context_t`, but is designed to grow as new subsystems are added. It is always stack-allocated in `kernel_main()` and its address is never stored globally — subsystem init functions receive pointers to the specific fields they need, not to the descriptor itself.

This design ensures that:

- Subsystem state ownership is explicit and visible in `kernel_main()`.
- No global mutable state is required.
- The lifetime of all kernel state is tied to the `kernel_main` stack frame.
