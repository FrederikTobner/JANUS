# Project Structure

Before we write a single line of assembly, let's talk organization. A well-structured OS project is the difference between "I know exactly where that code lives" and "I wrote this three weeks ago and now I can't find anything."

[!side]
Good structure pays dividends. You'll reference this code constantly as the project grows.
[/!side]

## Setting Up the Directory Structure

TinyOS follows a modular, library-based design inspired by the Linux kernel and LLVM.

First, let's create the project skeleton. Run these commands:

```bash
# Create the project root
mkdir -p TinyOS
cd TinyOS

# Create module directories
mkdir -p boot kernel include/tinyos cmake

# Create placeholder CMakeLists.txt files
touch boot/CMakeLists.txt
touch kernel/CMakeLists.txt
touch CMakeLists.txt
```

Your structure should now look like this:

```
TinyOS/
├── boot/                   # Boot module
│   └── CMakeLists.txt     # Boot build config
├── kernel/                 # Core kernel module
│   └── CMakeLists.txt     # Kernel build config
├── include/                # Standard library headers (freestanding)
│   └── tinyos/            # TinyOS-specific kernel headers
├── cmake/                  # Build system modules
│   ├── TinyOSPlatform.cmake
│   └── TinyOSHelpers.cmake
└── CMakeLists.txt          # Root build config
```

We will organize our code in different modules
Each module is self-contained with:

- Its own `CMakeLists.txt`
- An `include/` directory for public headers
- Implementation files
The only outlier here is the include directory that includes our header files.

## The Tiny Standard Library

Since we're in freestanding mode (no host OS), we can't use the system's C standard library. We provide our own with familiar names.

### Create `include/stdint.h`

```c-diff
file: include/stdint.h
replace: entire file
---
+#ifndef _STDINT_H
+#define _STDINT_H
+
+// Fixed-width integer types using compiler builtins
+typedef __UINT8_TYPE__  uint8_t;
+typedef __UINT16_TYPE__ uint16_t;
+typedef __UINT32_TYPE__ uint32_t;
+typedef __UINT64_TYPE__ uint64_t;
+
+typedef __INT8_TYPE__  int8_t;
+typedef __INT16_TYPE__ int16_t;
+typedef __INT32_TYPE__ int32_t;
+typedef __INT64_TYPE__ int64_t;
+
+// Limits
+#define UINT8_MAX  0xFF
+#define UINT16_MAX 0xFFFF
+#define UINT32_MAX 0xFFFFFFFF
+#define UINT64_MAX 0xFFFFFFFFFFFFFFFF
+
+#define INT8_MIN   (-128)
+#define INT8_MAX   127
+#define INT16_MIN  (-32768)
+#define INT16_MAX  32767
+#define INT32_MIN  (-2147483648)
+#define INT32_MAX  2147483647
+#define INT64_MIN  (-9223372036854775808L)
+#define INT64_MAX  9223372036854775807L
+
+#endif // _STDINT_H
```

`__UINT32_TYPE__` is a compiler builtin. Clang and GCC both provide these, making our headers portable.

### Create `include/stddef.h`

```c-diff
file: include/stddef.h
replace: entire file
---
+#ifndef _STDDEF_H
+#define _STDDEF_H
+
+// Basic type definitions
+typedef __SIZE_TYPE__ size_t;
+typedef __PTRDIFF_TYPE__ ptrdiff_t;
+
+#define NULL ((void*)0)
+#define offsetof(type, member) __builtin_offsetof(type, member)
+
+#endif // _STDDEF_H
```

### Create `include/stdbool.h`

```c-diff
file: include/stdbool.h
replace: entire file
---
+#ifndef _STDBOOL_H
+#define _STDBOOL_H
+
+#define bool  _Bool
+#define true  1
+#define false 0
+
+#define __bool_true_false_are_defined 1
+
+#endif // _STDBOOL_H
```

### Create `include/tinyos/types.h`

```c-diff
file: include/tinyos/types.h
replace: entire file
---
+#ifndef TINYOS_TYPES_H
+#define TINYOS_TYPES_H
+
+#include <stdint.h>
+
+// Semantic type aliases for kernel addresses
+typedef uint64_t phys_addr_t;  // Physical memory address
+typedef uint64_t virt_addr_t;  // Virtual memory address
+
+#endif // TINYOS_TYPES_H
```

**Why standard names?** Familiarity. Every C programmer knows `uint32_t` and `size_t`. Using standard interfaces means:

- Less mental overhead
- Better IDE autocomplete  
- Easier to apply existing C knowledge

**TinyOS-specific types** add semantic meaning:

```c
#include <stdint.h>
#include <tinyos/types.h>

phys_addr_t addr = 0x100000;  // "This is a physical address"
uint64_t count = 256;         // "This is just a number"
```

The type system prevents mistakes. You can't accidentally pass a physical address where a virtual address is expected—the compiler catches the type mismatch.

---

**Next: [The Multiboot2 Header →](multiboot-header.md)**
