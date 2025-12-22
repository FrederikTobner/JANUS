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

# Create kernel root
mkdir -p kernel
cd kernel

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

## Type definitions

Since we're in freestanding mode (no host OS), we can't use the system's C standard library. We provide our own with type definitions for commonly used types.

### Create `include/uapi/types.h`

```c-diff
file: include/types.h
replace: entire file
---

#ifndef _INT_LL64_H
#define _INT_LL64_H

typedef __UINT8_TYPE__   __u8;
typedef __UINT16_TYPE__  __u16;
typedef __UINT32_TYPE__  __u32;
typedef __UINT64_TYPE__  __u64;

typedef __INT8_TYPE__    __s8;
typedef __INT16_TYPE__   __s16;
typedef __INT32_TYPE__   __s32;
typedef __INT64_TYPE__   __s64;

#define UINT8_MAX   0xFF
#define UINT16_MAX  0xFFFF
#define UINT32_MAX  0xFFFFFFFFU
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define INT32_MIN   (-2147483648)
#define INT32_MAX   2147483647
#define INT64_MIN   (-9223372036854775807LL - 1)
#define INT64_MAX   9223372036854775807LL

#endif

```

We have placed it in uapi, so that we can expose it to the user space later.


### Create `include/tinyos/types.h`

```c-diff
file: include/tinyos/types.h
replace: entire file
---
+#ifndef TINYOS_TYPES_H
+#define TINYOS_TYPES_H
+
+#include "../uapi/typesh"
+
+typedef __s8   s8;
+typedef __u8   u8;
+typedef __s16 s16;
+typedef __u16 u16;
+typedef __s32 s32;
+typedef __u32 u32;
+typedef __s64 s64;
+typedef __u64 u64;
+
+#endif // TINYOS_TYPES_H
```

**Why not using standard names?**
We want to make the seperation of the kernel layer and the user space easily visible.

---

**Next: [The Multiboot2 Header →](multiboot-header.md)**
