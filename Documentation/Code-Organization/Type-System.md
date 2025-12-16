# Type System Organization

This document explains how types are organized across different headers in TinyOS.

## Three-Level Type Hierarchy

### Level 1: Basic Types (`include/stdint.h`)

**Purpose:** Freestanding replacement for standard C library type headers.

**Contents:**

- Fixed-width integer types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- Pointer-sized integers: `intptr_t`, `uintptr_t`
- NULL pointer definition
- Integer limits (`INT8_MIN`, `UINT32_MAX`, etc.)
- `offsetof` macro

**Replaces:** `<stdint.h>` from the standard library.

**Used by:** All kernel code that needs basic types.

### Level 2: Compiler Utilities (`/include/compiler.h`)

**Purpose:** Compiler-specific attributes and portable macros.

**Contents:**

- Compiler attributes: `__packed`, `__aligned`, `__noreturn`, `__unused`, `__used`, `__section`, `__weak`, `__always_inline`, `__noinline`, `__pure`, `__const`, `__cold`, `__hot`
- Branch prediction: `likely()`, `unlikely()`
- Memory barrier: `barrier()`
- Static assertion: `static_assert()`
- Utility macros: `ARRAY_SIZE()`, `container_of()`, `min()`, `max()`
- Compiler detection: `COMPILER_CLANG`, `COMPILER_GCC`

**Used by:** Code that needs compiler-specific features or optimization hints.

### Level 3: Kernel Types (`include/tinyos/types.h`)

**Purpose:** High-level kernel-specific type definitions.

**Contents:**

- Memory address types: `phys_addr_t`, `virt_addr_t`
- Page frame number: `pfn_t`
- Process/thread identifiers: `pid_t`, `tid_t`
- Device number: `dev_t`
- Error type: `error_t`

**Depends on:** `lib/types.h` (builds on top of basic types)

**Used by:** Kernel modules that work with memory management, processes, or devices.

## Include Patterns

### For Basic Operations (Low-Level Libraries)

```c
#include <lib/types.h>      // Basic types
#include <lib/compiler.h>   // Compiler attributes

// Now you have: uint8_t, size_t, bool, __packed, likely(), etc.
```

**Example:** `lib/memory/memory.c`, `lib/buffer/char_buffer.c`

### For Kernel-Level Code

```c
#include <tinyos/types.h>   // Includes lib/types.h automatically

// Now you have: phys_addr_t, virt_addr_t, pid_t, AND all basic types
```

**Example:** `mm/page_frame.c`, `kernel/main.c`, `arch/x86_64/cpu.c`

### For Configuration and Version Info

```c
#include <tinyos/config.h>  // Build configuration
#include <tinyos/version.h> // Version information
```

## Type Usage Guidelines

### When to Use Each Level

**Use `lib/types.h` when:**

- Writing low-level utility libraries
- You only need basic C types
- The code should be reusable outside the kernel context

**Use `tinyos/types.h` when:**

- Writing kernel-specific code
- Working with memory addresses, processes, or hardware
- You need kernel abstractions

**Use `lib/compiler.h` when:**

- You need compiler attributes (`__packed`, `__noreturn`, etc.)
- Optimizing performance with `likely()`/`unlikely()`
- Using utility macros like `ARRAY_SIZE()` or `container_of()`

### Type Semantics

**Memory Addresses:**

```c
phys_addr_t paddr;  // Physical address (hardware address)
virt_addr_t vaddr;  // Virtual address (program address)
```

These are both `uint64_t` but have different semantic meaning.

**Process Identifiers:**

```c
pid_t pid;  // Process ID (signed, allows -1 for error)
tid_t tid;  // Thread ID (signed, allows -1 for error)
```

**Error Codes:**

```c
error_t err;
// err < 0: Error occurred
// err == 0: Success
// err > 0: Informational (bytes read, etc.)
```

## Architecture-Specific Types (Future)

The directory structure includes `arch/include/arch/types.h` for architecture-specific type definitions. This will contain:

- Architecture-dependent alignment requirements
- Register types specific to x86_64
- Hardware descriptor table types (GDT, IDT, TSS)
- Control register types

**Not yet implemented** - will be added in future phases.

## Example Usage

### Low-Level Library (lib/memory/memory.c)

```c
#include <lib/types.h>
#include <lib/compiler.h>

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}
```

### Kernel Code (mm/page_frame.c)

```c
#include <tinyos/types.h>
#include <lib/compiler.h>

error_t mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags) {
    if (unlikely(!mm_is_page_aligned(vaddr))) {
        return -1;  // Error
    }
    
    // Map the page...
    return 0;  // Success
}
```

## Summary

| Header | Purpose | Provides | Depends On |
|--------|---------|----------|------------|
| `lib/types.h` | Basic C types | `uint8_t`, `size_t`, `bool`, etc. | None |
| `lib/compiler.h` | Compiler features | `__packed`, `likely()`, `ARRAY_SIZE()` | None |
| `tinyos/types.h` | Kernel types | `phys_addr_t`, `virt_addr_t`, `pid_t` | `lib/types.h` |
| `tinyos/config.h` | Build config | Version, compiler, build settings | None |
| `tinyos/version.h` | Version info | Version query functions | `tinyos/config.h` |

This three-level organization keeps low-level libraries independent while providing rich kernel-specific abstractions at higher levels.
