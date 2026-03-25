# Boot Context

`boot_context_t` is the single, protocol-agnostic data structure that carries all information gathered from the bootloader into the kernel. It is defined in `include/boot/context.h`.

## Data Flow

`kernel_main` allocates a `kernel_descriptor_t` on the stack, which embeds a `boot_context_t`. It then passes a pointer to the boot context into `boot_init()`. The protocol implementation reads firmware-provided data — Limine request responses or Multiboot2 tags — and populates every field of the context unconditionally.

After `boot_init()` returns, the boot context is fully populated and the rest of the init sequence can read framebuffer dimensions, the physical memory map, and other boot-time information through its fields.

## Source Files

| File | Description |
|------|-------------|
| `include/boot/context.h` | Structure definition and `boot_init()` prototype |

## Key Fields

The context captures:

- **Framebuffer geometry** — base address, width, height, pitch, bits per pixel, and RGB colour field positions.
- **Display availability** — `has_display` flag indicating whether a framebuffer was provided.
- **HHDM offset** — the higher-half direct map offset, used to translate physical addresses to virtual ones.
- **Boot protocol identity** — which protocol was used (Limine or Multiboot2).
