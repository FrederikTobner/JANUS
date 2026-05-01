# subsys — Kernel Subsystems

Kernel subsystems own hardware state and expose public C APIs. Each subsystem
is isolated: subsystems may not depend on other subsystems. Only `kmain` is
permitted to depend on multiple subsystems simultaneously.

Architecture-specific code lives inside the owning subsystem under `arch/`,
following the three-tier include hierarchy (Tier 1 public → Tier 2 contract
→ Tier 3 implementation).

## Subsystems

### boot

Protocol-agnostic boot context and protocol-specific initialization.
Defines `boot_context_t` and provides `boot_init()` — linked from the
correct protocol library (`boot_limine` or `boot_multiboot2`) per build.
See [boot/README.md](boot/README.md).

### drivers

Hardware-facing device drivers: serial (UART), TTY (text terminal +
framebuffer), and CPU control (`drivers_cpu_halt_forever`).
See [drivers/README.md](drivers/README.md).

### mm *(placeholder)*

Memory management subsystem. Will own the physical memory manager, virtual
memory manager, and page table lifecycle. Currently no sources.
See [mm/README.md](mm/README.md).
