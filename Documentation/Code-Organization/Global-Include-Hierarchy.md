# TinyOS Global Include Directory

## Purpose

The [`include/tinyos/`](../../include/tinyos/) directory contains **truly global** headers that are used across many modules and don't belong to any single module.

> **Rule**: If a header is only used by one or two modules, keep it in the module. Only promote to global if it's genuinely cross-cutting.

## What Goes in `include/tinyos/`

### Core Kernel Definitions
Headers that define fundamental kernel-wide concepts:

- **`types.h`** - Global type aliases and kernel-specific types
  - Wraps `lib/types.h` with kernel-specific additions
  - Physical/virtual address types
  - Page frame number types
  - Kernel-wide type aliases

- **`kernel.h`** - Core kernel macros and constants
  - Kernel version information
  - Build configuration
  - Kernel-wide constants (PAGE_SIZE, etc.)
  - Core kernel macros

- **`compiler.h`** - Compiler attributes
  - Usually just includes `lib/compiler.h`
  - Kernel-specific compiler attributes if needed

- **`errno.h`** - Error codes
  - System-wide error codes
  - Error code categories
  - Error translation functions

### System-Wide Interfaces
Headers that many subsystems need:

- **`printk.h`** - Kernel logging
  - printk() function (kernel printf)
  - Log levels (INFO, WARN, ERROR, DEBUG)
  - Early boot logging

- **`panic.h`** - Kernel panic handling
  - panic() function
  - assert() macros
  - BUG() and BUG_ON() macros

### Hardware Abstractions (Global)
Only truly platform-independent hardware abstractions:

- **`atomic.h`** - Atomic operations
  - Architecture-independent atomic interface
  - Actually implemented in arch/

- **`spinlock.h`** - Spinlocks
  - Spinlock types and operations
  - Lock debugging support

## What Does NOT Go in `include/tinyos/`

### Module-Specific Headers
These stay in their modules:

```
lib/buffer/include/lib/buffer.h     # Character buffers
lib/fio/include/lib/fio.h           # Formatted I/O
lib/memory/include/lib/memory.h     # Memory operations
arch/include/arch/cpu.h             # CPU operations
arch/include/arch/serial.h          # Serial port
mm/include/mm/page.h                # Page management
```

**Reasoning:** These are specific to one subsystem. Modules can include each other's headers when needed.

### Implementation Details
Internal headers never go in global include:

```
kernel/init_internal.h                  # Kernel internals
arch/x86_64/idt_internal.h             # IDT implementation details
```

**Reasoning:** Internal details should not be globally visible.

## Directory Structure

```
include/
├── tinyos/
│   ├── types.h           # Global types
│   ├── kernel.h          # Core kernel macros
│   ├── compiler.h        # Compiler attributes
│   ├── errno.h           # Error codes
│   ├── printk.h          # Kernel logging
│   ├── panic.h           # Panic handling
│   ├── atomic.h          # Atomic operations
│   └── spinlock.h        # Spinlocks
└── uapi/                 # User API (future)
    └── tinyos/
        └── syscall.h     # System call interface
```

## Include Patterns

### From Kernel Code

```c
// Global headers - angle brackets
#include <tinyos/kernel.h>
#include <tinyos/types.h>
#include <tinyos/printk.h>

// Library headers - angle brackets with module path
#include <lib/buffer.h>
#include <lib/fio.h>
#include <lib/memory.h>

// Architecture headers - angle brackets
#include <arch/cpu.h>
#include <arch/serial.h>

// Module-specific headers - angle brackets
#include <mm/page.h>

// Local module headers - quotes (same module)
#include "internal.h"
#include "module_internal.h"
```

### From Module Code

```c
// In lib/buffer/buffer.c
#include <lib/buffer.h>       // Own public header
#include <lib/memory.h>       // Other library
#include <lib/types.h>        # Base types
#include <tinyos/compiler.h>  // Global utilities
#include "buffer_internal.h"  // Own private header
```

## Example: `include/tinyos/types.h`

```c
#ifndef TINYOS_TYPES_H
#define TINYOS_TYPES_H

// Include base type definitions
#include <lib/types.h>

/**
 * Kernel-specific type definitions
 */

// Physical and virtual addresses
typedef uint64_t phys_addr_t;  // Physical address
typedef uint64_t virt_addr_t;  // Virtual address

// Page frame number
typedef uint64_t pfn_t;  // Page frame number

// Process/task IDs (future)
typedef int32_t pid_t;   // Process ID
typedef int32_t tid_t;   // Thread ID

// Device numbers (future)
typedef uint32_t dev_t;  // Device number

#endif // TINYOS_TYPES_H
```

## Example: `include/tinyos/kernel.h`

```c
#ifndef TINYOS_KERNEL_H
#define TINYOS_KERNEL_H

#include <tinyos/types.h>
#include <lib/compiler.h>

/**
 * Core kernel definitions
 */

// Kernel version
#define TINYOS_VERSION_MAJOR 0
#define TINYOS_VERSION_MINOR 1
#define TINYOS_VERSION_PATCH 0

// Page size (x86-64)
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define PAGE_MASK       (~(PAGE_SIZE - 1))

// Kernel address space (x86-64 higher half)
#define KERNEL_VIRT_BASE 0xFFFFFFFF80000000UL

// Alignment macros
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))

#define IS_ALIGNED(addr, align) (((addr) & ((align) - 1)) == 0)

// Page alignment
#define PAGE_ALIGN_DOWN(addr) ALIGN_DOWN(addr, PAGE_SIZE)
#define PAGE_ALIGN_UP(addr)   ALIGN_UP(addr, PAGE_SIZE)

#endif // TINYOS_KERNEL_H
```

## Example: `include/tinyos/printk.h`

```c
#ifndef TINYOS_PRINTK_H
#define TINYOS_PRINTK_H

#include <lib/compiler.h>

/**
 * Kernel logging functions
 */

// Log levels
#define KERN_EMERG   0  // System is unusable
#define KERN_ALERT   1  // Action must be taken immediately
#define KERN_CRIT    2  // Critical conditions
#define KERN_ERR     3  // Error conditions
#define KERN_WARNING 4  // Warning conditions
#define KERN_NOTICE  5  // Normal but significant
#define KERN_INFO    6  // Informational
#define KERN_DEBUG   7  // Debug messages

/**
 * Print formatted message to kernel log
 * 
 * @param fmt Printf-style format
 * @param ... Format arguments
 */
__attribute__((format(printf, 1, 2)))
void printk(char const * fmt, ...);

/**
 * Early boot printk (before full logging initialized)
 */
__attribute__((format(printf, 1, 2)))
void early_printk(char const * fmt, ...);

#endif // TINYOS_PRINTK_H
```

## When to Add New Global Headers

Ask these questions:

1. **Is it used by 3+ different modules?**
   - Yes → Consider global
   - No → Keep in module

2. **Does it define kernel-wide concepts?**
   - Yes → Global
   - No → Module-specific

3. **Is it an implementation detail?**
   - Yes → Keep private
   - No → Can be global

4. **Does it have dependencies on specific modules?**
   - Yes → Probably module-specific
   - No → Can be global

## Migration Policy

When a header becomes cross-cutting:

1. Create global version in `include/tinyos/`
2. Update documentation
3. Update all includes
4. Remove old module-specific version
5. Document in this file

Don't prematurely promote headers to global. Start module-specific, promote when needed.
