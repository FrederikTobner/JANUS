# Subsystems

## boot

Bridges firmware-provided boot data to the kernel's internal representation.
The `boot` subsystem is an INTERFACE library (headers only). The actual
initialisation logic lives in protocol-specific static libraries:

```
boot (INTERFACE)            headers: include/boot/context.h
  ├── boot_limine (STATIC)      protocol/limine/limine_boot.c
  └── boot_multiboot2 (STATIC)  protocol/multiboot2/multiboot2_boot.c (x86_64 only)
```

At link time exactly one protocol library is included. The linker resolves the
`boot_init` symbol to whichever protocol was selected.

`boot_init(boot_context_t *)` populates every field of `boot_context_t`
unconditionally to avoid uninitialized reads downstream. Supported protocols:

| Protocol | Architectures | Notes |
|---|---|---|
| Limine | x86_64, aarch64 | Assembly entry calls `kernel_main` directly |
| Multiboot2 | x86_64 | Assembly stashes magic/info pointer before `kernel_main` |

**Adding a protocol:** create `protocol/<name>/` with a CMakeLists.txt building a
static library that links `PUBLIC boot`, implement `boot_init()`, add assembly entry
points under `_start/<arch>/<name>/`, and wire the protocol into `cmake/boot/`.

## mm

Owns physical memory allocation. Currently provides a bitmap-based PMM covering
4 GiB of physical address space (128 KiB static bitmap, each bit = one 4 KiB page).

- All frames start marked used; `mm_pmm_init` marks usable regions free.
- The first 1 MiB is permanently reserved (BIOS, ISA DMA, VGA).
- The kernel image frames are punched out in a second pass.
- `mm_pmm_alloc_page()` returns `0` on OOM (physical address 0 is never a valid
  allocatable frame).

Virtual memory management and higher-level allocators are planned on top of this
foundation.

## core / kio

`kio` provides kernel formatted output (`kprintf`, `vkprintf`) and the panic
surface (`kpanic`). It bridges the `fmt` library to kernel output hardware via a
registered `putc` callback.

`kmain` registers the callback once console backends are initialised;
all other modules print or panic through the same interface.

`kpanic(msg, ...)` injects `__FILE__` and `__LINE__` at the call site via macro
and calls `kpanic_impl`, which prints a banner and halts the CPU via
`asm_cpu_halt_forever()`.

## drivers

Provides small, hardware-facing C APIs:

| Driver | Description |
|---|---|
| TTY | Text terminal — VGA text mode or framebuffer rendering |
| Serial | Character-level UART output (COM1 / PL011) |

Each driver exposes a narrow public interface through `include/drivers/` and hides
arch-specific implementation behind the three-tier hierarchy. The `tty` generic
layer handles cursor tracking, line wrapping, and scrolling; `arch_tty_*` functions
handle cell writing.

**Adding a driver:** add the public header to `include/drivers/`, add
arch-specific implementations under `arch/<arch>/`, add generic source to `src/`
if applicable.

## kmain

The composition root. Houses `kernel_main()`, the initialisation sequence, and
`kernel_descriptor_t` which threads boot-time data through to subsystem init
functions.

Init sequence:

1. `boot_init()` — parse firmware data into `boot_context_t`
2. `console_init()` — initialise serial and/or TTY, register `kio` callback
3. `mm_pmm_init()` — initialise physical frame allocator

`kmain` sits between `_start` (which calls `kernel_main`) and the subsystem layer.
It is the only module that links against subsystems, enforcing a star topology
where all inter-subsystem data flow is mediated through `kmain`.
