# Core Services

The core layer contains shared kernel services that are above `lib/` and below `subsys/`.

Core modules may depend on libraries, but they must not depend on subsystems. This keeps cross-cutting services reusable from every subsystem without creating subsystem-to-subsystem coupling.

## Components

| Component | Description |
|---|---|
| [kio](kio.md) | Kernel output and panic service (`kprintf`, `vkprintf`, `kpanic`) |

## Layering

```text
lib  ->  core  ->  subsys  ->  kmain
```

In practice, `kio` is the shared text output and fatal-error surface for the kernel. `kmain` registers the output callback once console backends are available, and all other modules can print or panic through the same interface.
