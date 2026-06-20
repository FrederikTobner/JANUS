# Core

The core layer contains shared kernel services that are used by multiple subsystems.

## Dependency Rules

- Core may depend on `lib/` modules.
- Core must not depend on `subsys/` modules.
- Subsystems may depend on core modules.

These rules are enforced by the CMake registry validation.

## Current Modules

- [kio](./kio/README.md) provides formatted output and panic service (`kprintf`, `vkprintf`, `kpanic`)
