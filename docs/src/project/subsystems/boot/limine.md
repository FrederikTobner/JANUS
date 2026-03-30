# Limine Protocol

The Limine boot protocol implementation populates the `boot_context_t` by reading Limine request/response structures. It supports both x86_64 and aarch64.

## Source Files

| File | Description |
|------|-------------|
| `protocol/limine/limine_boot.c` | `boot_init()` implementation — reads Limine responses |
| `protocol/limine/limine_protocol.h` | Limine request/response type definitions |

## How It Works

Limine uses a request/response model: the kernel places specially-tagged request structures in a dedicated linker section, and the bootloader fills in response pointers before transferring control. `limine_boot.c` reads these responses to populate the boot context.

Key requests used:

- **Framebuffer request** — provides the linear framebuffer address, dimensions, and pixel format.
- **HHDM request** — provides the higher-half direct map offset.
- **Memory map request** — provides the physical memory layout.

## Assembly Entry

The Limine entry point is minimal — the bootloader already sets up a 64-bit environment with paging enabled, so the assembly stub simply calls `kernel_main` directly.
