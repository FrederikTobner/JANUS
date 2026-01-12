# Project Structure

Before we write a single line of code, we should first define how we will organize our project. 
TinyOS follows a modular, library-based design inspired by the Linux kernel and LLVM.
First, let's create the project skeleton. Run these commands:

```bash
# Create the project root
mkdir -p TinyOS
cd TinyOS

# Create kernel root and cmake directories
mkdir -p kernel cmake
cd kernel

# Create module directories
mkdir -p boot kernel include/tinyos include/uapi 

# Create placeholder CMakeLists.txt files
touch boot/CMakeLists.txt
touch kernel/CMakeLists.txt
touch CMakeLists.txt
```

Your structure should now look like this:

```
TinyOS/
├── cmake/                        # Build system modules
│   ├── TinyOSPlatform.cmake 
│   └── TinyOSHelpers.cmake
├── CMakeLists.txt                # Root build config
└── kernel
    ├── boot/                     # Boot module
    │   └── CMakeLists.txt        # Boot build config
    ├── core/                     # Core kernel module
    │   └── CMakeLists.txt        # Kernel build config
    └── include/                  # Global headers of our kernel
        ├── uapi/                 # User API headers
        └── tinyos/               # TinyOS-specific kernel headers
```

We will organize our code in different modules
Each module is self-contained with it's own `CMakeLists.txt`, and an `include/` directory for public headers.

[!side]
Freestanding headers are headers that do not depend on any underlying operating system or standard library.
These are for example `<stdint.h>`, `stddef.h`, or `stdbool.h`.
[/!side]

Since we're in freestanding mode, meaning there is no host OS, we can't use everything the system's C standard library provides. 
We provide our own with type definitions for commonly used types.

First lets create a file where we will define all the types that we expose to the user-space. 

```c-diff
file: include/uapi/types.h
replace: entire file
---
+#ifndef KERNEL_UAPI_TYPES_H
+#define KERNEL_UAPI_TYPES_H
+
+typedef __UINT8_TYPE__   __u8;
+typedef __UINT16_TYPE__  __u16;
+typedef __UINT32_TYPE__  __u32;
+typedef __UINT64_TYPE__  __u64;
+
+typedef __INT8_TYPE__    __s8;
+typedef __INT16_TYPE__   __s16;
+typedef __INT32_TYPE__   __s32;
+typedef __INT64_TYPE__   __s64;
+#endif
```

Now lets create `include/tinyos/types.h`

```c-diff
file: include/tinyos/types.h
replace: entire file
---
+#ifndef KERNEL_GLOBAL_TYPES_H
+#define KERNEL_GLOBAL_TYPES_H
+
+#include <uapi/types.h>
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
+#endif 
```

These type defintions we will use throughout the kernel.

Maybe you are asking youself why we are not using standard names or include `stdint.h`, since it is a freestanding header.
We want to make the seperation of the kernel layer and the user space clearly visible.
Using the fixed-integer types from stdint.h in the kernel code would blur this line.

---

**Next: [The Multiboot2 Header →](multiboot-header.md)**
