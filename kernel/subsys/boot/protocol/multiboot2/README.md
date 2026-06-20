# Multiboot2

- `multiboot2_boot.c` — implements `boot_init()` and `multiboot2_stash_bootinfo()`:
  the stash function saves the magic/info pointer from CPU registers (called by
  assembly before `kernel_main`), then `boot_init()` validates and parses the
  Multiboot2 tag list
- `multiboot2_protocol.h` — Multiboot2 structures, tag constants, and tag
  iteration helpers
