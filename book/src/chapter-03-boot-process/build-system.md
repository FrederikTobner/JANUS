# The CMake Build System

We have source files. We have a linker script. But manually assembling and linking everything is tedious and error-prone. Time to automate.

> **The Crux: Why Not Just a Shell Script?**
>
> You could write a bash script: `nasm this, clang that, ld the other`. It would work! For about a week. Then you add a new source file and forget to update the script. Or you want debug builds vs. release builds. Or you work on macOS and your teammate uses Linux. Shell scripts don't scale.
>
> CMake handles dependency tracking, parallel builds, cross-platform differences, and more. It's the industry standard for good reason.

[!side]
The Linux kernel uses Makefiles directly. We use CMake because it's easier to learn and more portable.
[/!side]

## Build System Architecture

TinyOS utilizes a modular CMake structure where each component is self-contained:

```
build system/
├── CMakeLists.txt (root)           # Orchestrates everything
├── cmake/
│   ├── TinyOSPlatform.cmake       # Compiler detection, flags
│   └── TinyOSHelpers.cmake        # Helper functions
├── boot/CMakeLists.txt            # Boot module
└── kernel/CMakeLists.txt          # Final kernel executable
```

**Why modular?** Each module builds independently. Change one file in `boot/`, only that library rebuilds. The kernel just re-links. Fast incremental builds.

## Platform Detection

**File:** `cmake/TinyOSPlatform.cmake`

This module detects your environment and sets up cross-compilation:

- **Host platform**: Linux, macOS, or Windows
- **Compiler**: Clang (preferred) or GCC
- **Target**: Always x86_64-elf (freestanding)
- **Build type**: Debug (with `-g3 -O0`) or Release (`-O2`)

**Key flags for kernel code:**

```cmake
-target x86_64-elf      # Cross-compile to bare metal
-nostdlib               # No standard library
-ffreestanding          # Freestanding environment
-mno-red-zone           # Disable red zone (x86-64 ABI quirk)
-fno-stack-protector    # No stack canaries (we're the OS!)
```

[!side]
Stack protectors insert canary values to detect buffer overflows. Requires runtime support we don't have yet.
[/!side]

> **Implementation detail:**
>
> TinyOSPlatform.cmake sets these flags in variables like `TINYOS_COMMON_FLAGS`, `TINYOS_DEBUG_FLAGS`, etc. The helper functions in TinyOSHelpers.cmake apply them to targets automatically. This keeps platform-specific logic centralized—if you port to ARM later, you only change one file.
>
> For Chapter 3, you can create a minimal `TinyOSPlatform.cmake` that just sets the flags directly with `add_compile_options()`. We'll expand it in later chapters when we add more architecture-specific code.

Here's a minimal platform module to get started:

```cmake-diff
file: cmake/TinyOSPlatform.cmake
after: entire file
---
+# cmake/TinyOSPlatform.cmake
+# Minimal platform detection and compiler setup for Chapter 3
+
+# Kernel compile flags (apply to all targets)
+add_compile_options(
+    -target x86_64-elf
+    -ffreestanding
+    -nostdlib
+    -mno-red-zone
+    -fno-stack-protector
+    -Wall -Wextra
+)
+
+# Debug vs Release
+if(CMAKE_BUILD_TYPE STREQUAL "Debug")
+    add_compile_options(-g3 -O0)
+else()
+    add_compile_options(-O2)
+endif()
```

This applies kernel flags to everything. As the project grows, you'll refactor this into finer-grained control (some libraries might not need `-ffreestanding`, for example).

## Helper Functions

Create `cmake/TinyOSHelpers.cmake` with helper functions for kernel development:

```cmake-diff
file: cmake/TinyOSHelpers.cmake
replace: entire file
---
+# cmake/TinyOSHelpers.cmake
+
+# Create kernel executable with custom linker script
+function(tinyos_create_kernel)
+    cmake_parse_arguments(
+        ARG
+        ""
+        "LINKER_SCRIPT"
+        "SOURCES;LIBRARIES"
+        ${ARGN}
+    )
+    
+    # Create the kernel executable
+    add_executable(kernel.elf ${ARG_SOURCES})
+    
+    # Link with provided libraries and objects
+    if(ARG_LIBRARIES)
+        target_link_libraries(kernel.elf PRIVATE ${ARG_LIBRARIES})
+    endif()
+    
+    # Apply linker script
+    if(ARG_LINKER_SCRIPT)
+        target_link_options(kernel.elf PRIVATE
+            -T ${ARG_LINKER_SCRIPT}
+            -nostdlib
+            -static
+        )
+        # Ensure rebuild when linker script changes
+        set_target_properties(kernel.elf PROPERTIES
+            LINK_DEPENDS ${ARG_LINKER_SCRIPT}
+        )
+    endif()
+    
+    # Set kernel-specific flags
+    target_compile_options(kernel.elf PRIVATE
+        -ffreestanding
+        -fno-stack-protector
+        -mno-red-zone
+    )
+    
+    # Include directories
+    target_include_directories(kernel.elf PRIVATE
+        ${CMAKE_SOURCE_DIR}/include
+        ${CMAKE_SOURCE_DIR}/boot/include
+    )
+endfunction()
```

Key functions:

**`tinyos_create_kernel(SOURCES ... LIBRARIES ... LINKER_SCRIPT ...)`**

- Creates final `kernel.elf` executable
- Links all libraries and boot objects
- Applies custom linker script
- Sets entry point to `_start`

## Configuration Header Template

Before diving into the root build script, let's create a configuration header template. This is a neat CMake trick: you create a `.in` file with placeholders that CMake fills in at build time.

Create `include/tinyos/config.h.in`:

```cmake-diff
file: include/tinyos/config.h.in
after: entire file
---
+/**
+ * @file config.h
+ * @brief TinyOS build configuration (generated from config.h.in by CMake)
+ * 
+ * This file is automatically generated during the build process.
+ * DO NOT EDIT THIS FILE DIRECTLY - edit config.h.in instead.
+ */
+
+#ifndef TINYOS_CONFIG_H
+#define TINYOS_CONFIG_H
+
+/*
+ * Project Information
+ */
+#define TINYOS_PROJECT_NAME     "@PROJECT_NAME@"
+#define TINYOS_VERSION_MAJOR    @PROJECT_VERSION_MAJOR@
+#define TINYOS_VERSION_MINOR    @PROJECT_VERSION_MINOR@
+#define TINYOS_VERSION_PATCH    @PROJECT_VERSION_PATCH@
+#define TINYOS_VERSION_STRING   "@PROJECT_VERSION@"
+
+/*
+ * Build Information
+ */
+#define TINYOS_BUILD_TYPE       "@CMAKE_BUILD_TYPE@"
+#define TINYOS_BUILD_DATE       "@TINYOS_BUILD_DATE@"
+#define TINYOS_BUILD_TIME       "@TINYOS_BUILD_TIME@"
+
+/*
+ * Target Architecture
+ */
+#define TINYOS_ARCH_X86_64      1
+#define TINYOS_TARGET_ARCH      "x86_64"
+
+#endif /* TINYOS_CONFIG_H */
```

This template uses CMake variables surrounded by `@` symbols. When you call `configure_file()`, CMake replaces them with actual values:

- `@PROJECT_NAME@` → `"TinyOS"`
- `@PROJECT_VERSION_MAJOR@` → `0`
- `@TINYOS_BUILD_DATE@` → `"2025-01-15"` (current date)

The generated `config.h` lives in your build directory and gets included automatically.

> **Why not just hardcode values?**
>
> Because they'd go stale. Every time you bump the version or switch between debug/release builds, you'd manually edit a header. With templates, CMake keeps everything synchronized automatically. Change `project(TinyOS VERSION 0.2.0)` and your kernel instantly knows its new version number.

## Building the Kernel

Now create the root `CMakeLists.txt` that generates the config header and orchestrates the build:

```cmake-diff
file: CMakeLists.txt
replace: entire file
---
+# CMakeLists.txt (root)
+cmake_minimum_required(VERSION 3.20)
+project(TinyOS VERSION 0.1.0 LANGUAGES ASM_NASM C)
+
+# Set C standard
+set(CMAKE_C_STANDARD 11)
+set(CMAKE_C_STANDARD_REQUIRED ON)
+
+# Add our CMake modules to the search path
+list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
+
+# Include platform detection and helpers
+include(TinyOSPlatform)
+include(TinyOSHelpers)
+
+# Generate build timestamp
+string(TIMESTAMP TINYOS_BUILD_DATE "%Y-%m-%d")
+string(TIMESTAMP TINYOS_BUILD_TIME "%H:%M:%S")
+
+add_subdirectory(kernel)
```

Lets also print the build cnfiguration afterwards:

```cmake-diff
file: CMakeLists.txt
after: project(TinyOS VERSION 0.1.0 LANGUAGES C ASM_NASM)
---
cmake_minimum_required(VERSION 3.20)
project(TinyOS VERSION 0.1.0 LANGUAGES C ASM_NASM)

+message(STATUS "========================================")
+message(STATUS "TinyOS Build Configuration")
+message(STATUS "========================================")
+message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
+message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
+message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER}")
+message(STATUS "========================================")
```

After that we create the `CMakeLists.txt`  in the kernel folder:

```cmake-diff
file: kernel/CMakeLists.txt
replace: entire file
---
configure_file(
    "${CMAKE_SOURCE_DIR}/include/tinyos/config.h.in"
    "${CMAKE_BINARY_DIR}/include/tinyos/config.h"
    @ONLY
)

include_directories("${CMAKE_BINARY_DIR}/kernel/include")

add_subdirectory(boot)
add_subdirectory(core)

# Generate grub.cfg from template
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in
    ${CMAKE_BINARY_DIR}/kernel/grub.cfg
    @ONLY
)

```

[/side]
The timestamp is generated when you run `cmake -B build`, not when you run `ninja`. If you want per-build timestamps, you'd need a custom command that runs on every build.
For our small project, knowing when the build system was configured will be sufficient.
[!/side]

**Configure:**

```bash
cmake -B build -G Ninja
```

**Build:**

```bash
ninja -C build
```

You'll see the build progress:

```
ninja: Entering directory `build'
[1/7] Building ASM_NASM object boot/CMakeFiles/boot.dir/multiboot.asm.o
[2/7] Building ASM_NASM object boot/CMakeFiles/boot.dir/boot.asm.o
[3/7] Building C object lib/types/CMakeFiles/types.dir/types.c.o
[4/7] Linking C static library lib/types/libtypes.a
[5/7] Building C object lib/memory/CMakeFiles/memory.dir/memory.c.o
[6/7] Linking C static library lib/memory/libmemory.a
[7/7] Linking C executable kernel.elf
```

This produces `build/kernel.elf` - a bootable ELF executable.

## Build Flow

```
                    Source Files
                        │
        ┌───────────────┼───────────────┐
        │               │               │
        ▼               ▼               ▼
   multiboot.asm   boot.asm        main.c, lib/*.c
        │               │               │
        ▼               ▼               ▼
     [NASM]          [NASM]         [Clang]
        │               │               │
        ▼               ▼               ▼
   multiboot.o      boot.o           *.o files
        │               │               │
        └───────┬───────┴───────┬───────┘
                │               │
                ▼               ▼
            boot.a          lib*.a
                │               │
                └───────┬───────┘
                        ▼
                   [Linker LD]
                   (linker.ld)
                        │
                        ▼
                  kernel.elf ✓
                (Bootable!)
```

> **TODO: Hand-drawn illustration idea**
> Draw a factory assembly line where source files (.asm, .c) are raw materials on a conveyor belt. NASM and Clang are workers at stations with hammers/tools transforming them into .o files (boxes). These boxes all funnel into a big "Linker" machine that looks like it's under pressure (steam coming out, gears turning), and out pops a shiny kernel.elf with a "Grade A Bootable" stamp and a smiley face.

**Parallel builds:** Ninja automatically compiles modules in parallel. On a 4-core machine, this is 3-4× faster than sequential builds.

**Incremental builds:** Change one `.c` file, only that file and the final link step re-run. No wasted rebuilds.

## Verification

> TODO: Is this possible without kernel entry point?

After building, verify the kernel:

```bash
# Should be ELF 64-bit executable
file build/kernel.elf
```

Output:

```
build/kernel.elf: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), 
statically linked, with debug_info, not stripped
```

> **New to ELF?**
>
> ELF (Executable and Linkable Format) is the standard executable format on Unix-like systems (Linux, BSD, etc.). It's a container that holds:
>
> - **Machine code** (your compiled program)
> - **Section headers** (where different parts of code/data live)
> - **Program headers** (how to load the file into memory)
> - **Symbol tables** (function names, variable names for debugging)
>
> The `.elf` extension is optional—on Linux, executables typically have no extension. We use it to make it clear this is not a normal userspace program but a freestanding kernel. GRUB knows how to parse ELF files and load them according to their program headers.

Check the entry point:

```bash
readelf -h build/kernel.elf | grep Entry
```

Output:

```
  Entry point address:               0x101000
```

Check load addresses:

```bash
readelf -l build/kernel.elf | grep LOAD
```

Output:

```
  LOAD           0x0000000000001000 0x0000000000100000 0x0000000000100000
  LOAD           0x0000000000002000 0x0000000000101000 0x0000000000101000
  LOAD           0x0000000000003000 0x0000000000102000 0x0000000000102000
  LOAD           0x0000000000000000 0x0000000000103000 0x0000000000103000
```

**What this shows:** All LOAD segments have virtual and physical addresses starting at 0x100000 (1MB).

**Check for Multiboot2 header:**

```bash
readelf -S build/kernel.elf | grep multiboot
```

Output:

```
  [ 1] .multiboot        PROGBITS         0000000000100000  00001000
```

**What this shows:** The `.multiboot` section is at address 0x100000, the very first byte of our kernel image.

---

**Next: [The Kernel Entry Point →](kernel-entry.md)**
