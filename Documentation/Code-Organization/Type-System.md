# Type System Organization

This document explains how types are organized across different headers in TinyOS.

## Three-Level Type Hierarchy

### Level 1: Basic Types (`kernel/include/tinyos/types.h`)

**Purpose:** Freestanding replacement for standard C library type headers.

**Contents:**

- Fixed-width integer types: `u8`, `s8`, `u16`, `u16`, `u32`, `u32`, `u64`, `u64`

**Used by:** All kernel code that needs basic types.

### Level 2: Compiler Utilities (`/kernel/include/tinyos/compiler.h`)

**Purpose:** Compiler-specific attributes and portable macros.

**Contents:**

- Compiler attributes: `__packed`, `__aligned`, `__noreturn`, `__unused`, `__used`, `__section`, `__weak`, `__always_inline`, `__noinline`, `__pure`, `__const`, `__cold`, `__hot`
- Branch prediction: `likely()`, `unlikely()`
- Memory barrier: `barrier()`
- Static assertion: `static_assert()`
- Utility macros: `ARRAY_SIZE()`, `container_of()`, `min()`, `max()`
- Compiler detection: `COMPILER_CLANG`, `COMPILER_GCC`

**Used by:** Code that needs compiler-specific features or optimization hints.

### Level 3: User API Types (`/kernel/include/uapi/types.h`)

**Purpose:** Exposes kernel-specific types to user programs.

