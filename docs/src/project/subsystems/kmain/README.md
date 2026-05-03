# Kernel Main

`kmain` is the composition root of the kernel. It is the only module permitted to depend on subsystems, and it owns the initialisation sequence that brings the kernel from raw boot data to a running state.

## Components

| Component | Description |
|-----------|-------------|
| [Main](main.md) | `kernel_main()` entry point and top-level flow |
| [Init](init.md) | Subsystem initialisation functions |
| [Kernel Descriptor](kernel-descriptor.md) | `kernel_descriptor_t` — the top-level state aggregate |

## Layering

`kmain` sits between the assembly entry points (the `_start` layer) and the subsystem layer. It is the sole module that links against subsystems, enforcing a star topology where all inter-subsystem data flow is mediated through `kmain` rather than through direct subsystem-to-subsystem calls.

```text
_start  →  kernel_main (kmain)  →  boot, drivers, mm (subsys) + kio (core)
```
