# TinyOS Global Include Directory

## Purpose

The [`include/tinyos/`](../../include/tinyos/) directory contains **truly global** headers that are used across many modules and don't belong to any single module.

> **Rule**: If a header is only used by a couple of kernel modules, keep it in a kernel library. Only promote to global if it's genuinely cross-cutting.

## What Goes in `kernel/include/tinyos/`

### Core Kernel Definitions

Headers that define fundamental kernel-wide concepts:

- **`types.h`** - Global type aliases and kernel-specific types
  - Fixed-width integer types (u8, s8, u16, u32, u64)

- **`kernel.h`** - Core kernel macros and constants

- **`compiler.h`** - Compiler attributes

### Kernel Utility Libraries

For functionality used by many kernel modules we are using the `/kernel/lib` directory.
Kernel utility libraries are isolated from each other and are not allowed to depend on any other kernel libraries or kernel modules.
They are only allowed to use our core kernel definitions from `kernel/include/tinyos/`.

### Kernel Modules

Kernel modules are allowed to use kernel utility libraries and core kernel definitions.
But they are not allowed to depend on other kernel modules. These kernel modules are isolated building blocks that can be composed to form the kernel.

### Kernel Core Module

This is the only kernel module allowed to depend on other kernel modules.
It is the entry point of the kernel and is responsible for initializing and coordinating all other kernel modules.
It lives in `kernel/core/`.

### User API Definitions

> **TODO:** Add user API defiinition once it is ready.
