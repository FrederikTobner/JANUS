# Limine

- `limine_boot.c` — implements `boot_init()`: reads Limine request responses
  (HHDM, executable address, framebuffer) and populates the boot context
- `limine_protocol.h` — Limine protocol structures and request declarations

Limine request symbols are defined in `_start/common/limine_requests.c` (shared
across architectures).
