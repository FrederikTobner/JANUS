# JANUS — Overview

JANUS is a freestanding kernel written in C using the C17 standard, targeting the x86_64 and aarch64 architectures.

## Design Principles

### Strict Layering

JANUS is organised into layers with one-directional dependencies.
No module may depend on anything in the same layer or above it.
Any violations are already caught by CMake at configuration time.

A detailed view of the dependency graph is generated automatically during each CMake configure run and written to `docs/generated/`.

### Modular, Co-located Architecture

Each module has clear boundaries, explicit dependencies, and a public header interface that is the sole point of contact with the rest of the kernel.
Rather than centralising all platform code in a single `arch/` tree, like it is done in the Linux kernel, every subsystem and library contains its own `arch/` subdirectory.
This means a module's complete implementation, generic logic and platform-specific code alike, is navigable as a single unit without jumping between distant directories.
