# JANUS Kernel Main Module

Core kernel initialization and orchestration.

## Responsibilities

- Allocate the `kernel_descriptor_t` on the stack (aggregates all boot-time data)
- Call `boot_init()` to populate the boot context
- Initialize subsystems in the correct order (serial, TTY)
- Print the kernel banner
- Halt the CPU

## Key Files

- `src/main.c` — `kernel_main()` entry point
- `src/init.c` — Subsystem initialization routines (`kinit_serial`, `kinit_tty`)
- `src/banner.c` — Kernel banner printing
- `internal/kmain/kernel_descriptor.h` — `kernel_descriptor_t` (kmain-private)
- `internal/kmain/init.h` — Internal init function declarations
- `internal/kmain/banner.h` — Internal banner function declarations

## Layering

`kmain` is the **only** module allowed to depend on subsystems. It sits between
the assembly entry points (`_start`) and the subsystem layer (`subsys/`).

```text
_start → kernel_main (kmain) → boot, drivers (subsys) → page_tables (lib)
```
