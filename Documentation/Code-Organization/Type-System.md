# Type System Organization

This document explains how types are organized across different headers in TinyOS.

## Three-Level Type Hierarchy

### Level 1: User API Types (`/kernel/include/uapi/types.h`)

**Purpose:** Exposes kernel-specific types to user programs.

**Contents:**

- Fixed-width integer types with `__` prefix (e.g., `__u8`, `__s8`, `__u16`, etc.)
### Level 2: Basic Types (`kernel/include/tinyos/types.h`)

**Purpose:** Freestanding replacement for standard C library type headers.

**Contents:**

- Fixed-width integer types: `u8`, `s8`, `u16`, `u16`, `u32`, `u32`, `u64`, `u64`

**Used by:** All kernel code that needs basic types.

