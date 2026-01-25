# The CMake Build System

We have source files. We have a linker script. But manually assembling and linking everything is tedious and error-prone. Time to automate.

> **The Crux: Why Not Just a Shell Script?**
>
> You could write a bash script: `running nasm on our assembly files, clang on our C files, and then use ld to create the ELF file`. It would work! For about a week. Then you add a new source file and forget to update the script. Or you want debug builds vs. release builds. Or you work on multiple systems with different compilers, linkers or a different underlying build system, like Make instead of Ninja. Shell scripts don't scale well in that regard.
> CMake handles dependency tracking, parallel builds, cross-platform differences, and allows you to change the underlying build system.
> For that reason it is called a meta build system. It's the industry standard for good reason.
> Newer meta build systems like Meson and Bazel exist, but CMake is still the most widely used when working with C. Therefore, we'll use it for TinyOS.

[!side]
The Linux kernel uses Makefiles directly. We use CMake because it's easier to learn and more portable.
[/!side]

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
Additionally this will help to keep the complexity manageable as the project grows.

## Platform Detection

First we'll create a separate CMake module for platform detection and compiler setup.

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
-fno-stack-protector    # Dont setup a stack protector
-fno-stack-protector    # Don't set up a stack protector
```

[!side]
Stack protectors insert canary values in order to detect buffer overflows.
Which is neat, but they require runtime support that we don't have in a freestanding environment.
[/!side]

> **Implementation detail:**
>
> TinyOSPlatform.cmake sets these flags in variables like `TINYOS_COMMON_FLAGS`, `TINYOS_DEBUG_FLAGS`, etc. The helper functions in TinyOSHelpers.cmake apply them to targets automatically. This keeps platform-specific logic centralized—if you port to ARM later, you only change one file.
>
> For now, let's create a minimal `TinyOSPlatform.cmake` that just sets the flags directly with `add_compile_options()`. We'll expand it in later chapters when we add more architecture-specific code.

> TODO: Make a little bit less minimal, but without adding all the complexity from the real repo.

Here's a minimal platform module to get started:

```cmake-diff
file: cmake/TinyOSPlatform.cmake
after: entire file
---
+ include_guard(GLOBAL)
+ 
+ if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
+     set(TINYOS_HOST_LINUX TRUE)
+ elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
+     set(TINYOS_HOST_WINDOWS TRUE)
+ elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
+     set(TINYOS_HOST_MACOS TRUE)
+ else()
+     message(WARNING "Unknown host platform: ${CMAKE_SYSTEM_NAME}")
+ endif()
+ 
+ set(TINYOS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
+ set(TINYOS_TARGET_PLATFORM "elf")
+ 
+ if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
+     set(TINYOS_COMPILER_CLANG TRUE)
+     set(TINYOS_TARGET_FLAG "-target")
+ elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
+     set(TINYOS_COMPILER_GCC TRUE)
+     set(TINYOS_TARGET_FLAG "--target=")
+ else()
+     message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. TinyOS requires Clang or GCC.")
+ endif()
+ 
+ if(NOT CMAKE_BUILD_TYPE)
+     set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, MinSizeRel)" FORCE)
+ endif()
+ 
+ message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
+ message(STATUS "Target: ${TINYOS_TARGET_ARCH}-${TINYOS_TARGET_PLATFORM}")
+ 
+ set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)
+ 
+ if(TINYOS_COMPILER_CLANG)
+     set(TINYOS_COMMON_FLAGS
+         -target x86_64-elf
+         -nostdlib
+         -ffreestanding
+         -fno-builtin
+         -fno-stack-protector
+         -mno-red-zone
+         -Wall
+         -Wextra
+         -Werror
+         -Wconversion
+         -Wimplicit
+         -Wcast-qual
+         -Wpointer-arith
+     )
+ else()
+     set(TINYOS_COMMON_FLAGS
+         -nostdlib
+         -ffreestanding
+         -fno-builtin
+         -fno-stack-protector
+         -mno-red-zone
+         -Wall
+         -Wextra
+         -Werror
+         -Wconversion
+         -Wimplicit
+         -Wcast-qual
+         -Wpointer-arith
+     )
+ endif()
+ 
+ set(TINYOS_DEBUG_FLAGS
+     -g3
+     -gdwarf-4
+     -O0
+     -DDEBUG
+ 
+ )
+ 
+ set(TINYOS_RELEASE_FLAGS
+     -O2
+     -DNDEBUG
+ )
+ 
+ set(TINYOS_MINSIZEREL_FLAGS
+     -Os
+     -DNDEBUG
+ )
+ 
+ set(TINYOS_PLATFORM_LOADED TRUE)
```

This applies kernel flags to everything. As the project grows, you'll refactor this into finer-grained control (some libraries might not need `-ffreestanding`, for example).

## Helper Functions

Create `cmake/TinyOSHelpers.cmake` with helper functions for kernel development:

```cmake-diff
file: cmake/TinyOSHelpers.cmake
replace: entire file
---
+  include_guard(GLOBAL)
+  
+  if(NOT TINYOS_PLATFORM_LOADED)
+      message(FATAL_ERROR "TinyOSPlatform.cmake must be included before TinyOSHelpers.cmake")
+  endif()
+  
+  #
+  # Add a kernel library with standard configuration
+  # 
+  # Usage:
+  #   tinyos_add_library(name
+  #       SOURCES file1.c file2.c
+  #       [DEPENDENCIES dep1 dep2]
+  #   )
+  #
+  function(tinyos_add_library NAME)
+      cmake_parse_arguments(
+          ARG                    # Prefix for parsed arguments
+          ""                     # Options (boolean flags)
+          ""                     # Single-value arguments
+          "SOURCES;DEPENDENCIES" # Multi-value arguments
+          ${ARGN}
+      )
+  
+      if(NOT ARG_SOURCES)
+          message(STATUS "  Added library: ${NAME} (placeholder, no sources yet)")
+          add_library(${NAME} INTERFACE)
+          
+          target_include_directories(${NAME}
+              INTERFACE
+                  ${CMAKE_CURRENT_SOURCE_DIR}/include
+                  ${CMAKE_SOURCE_DIR}/include
+          )
+          
+          if(ARG_DEPENDENCIES)
+              target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
+          endif()
+          
+          return()
+      endif()
+  
+      add_library(${NAME} STATIC ${ARG_SOURCES})
+  
+      target_include_directories(${NAME}
+          PUBLIC
+              ${CMAKE_CURRENT_SOURCE_DIR}/include
+              ${CMAKE_SOURCE_DIR}/include
+      )
+  
+      if(ARG_DEPENDENCIES)
+          target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
+      endif()
+  
+      target_compile_options(${NAME} PRIVATE ${TINYOS_COMMON_FLAGS})
+      
+      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_DEBUG_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_RELEASE_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
+      endif()
+  
+      message(STATUS "  Added library: ${NAME}")
+  endfunction()
+  
+  #
+  # Add a kernel module (like kernel, boot, arch, mm)
+  # 
+  # Usage:
+  #   tinyos_add_module(name
+  #       SOURCES file1.c file2.c
+  #       [DEPENDENCIES dep1 dep2]
+  #   )
+  #
+  function(tinyos_add_module NAME)
+      cmake_parse_arguments(
+          ARG
+          ""
+          ""
+          "SOURCES;DEPENDENCIES"
+          ${ARGN}
+      )
+  
+      # Validate required arguments
+      if(NOT ARG_SOURCES)
+          message(STATUS "  Added module: ${NAME} (placeholder, no sources yet)")
+          add_library(${NAME} INTERFACE)
+          
+          target_include_directories(${NAME}
+              INTERFACE
+                  ${CMAKE_CURRENT_SOURCE_DIR}/include
+                  ${CMAKE_SOURCE_DIR}/kernel/include
+          )
+          
+          if(ARG_DEPENDENCIES)
+              target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
+          endif()
+          
+          return()
+      endif()
+  
+      add_library(${NAME} STATIC ${ARG_SOURCES})
+  
+      target_include_directories(${NAME}
+          PUBLIC
+              ${CMAKE_CURRENT_SOURCE_DIR}/include
+              ${CMAKE_SOURCE_DIR}kernel/include
+      )
+  
+      if(ARG_DEPENDENCIES)
+          target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
+      endif()
+  
+      target_compile_options(${NAME} PRIVATE ${TINYOS_COMMON_FLAGS})
+      
+      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_DEBUG_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_RELEASE_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
+          target_compile_options(${NAME} PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
+      endif()
+  
+      message(STATUS "  Added module: ${NAME}")
+  endfunction()
+  
+  # Print TinyOS build configuration summary
+  #
+  # Create the final kernel executable with custom linker script
+  # 
+  # Usage:
+  #   tinyos_create_kernel(
+  #       SOURCES main.c init.c
+  #       LIBRARIES lib1 lib2
+  #       LINKER_SCRIPT path/to/linker.ld
+  #   )
+  #
+  function(tinyos_create_kernel)
+      cmake_parse_arguments(
+          ARG
+          ""
+          "LINKER_SCRIPT"
+          "SOURCES;LIBRARIES"
+          ${ARGN}
+      )
+  
+      if(NOT ARG_SOURCES)
+          message(FATAL_ERROR "tinyos_create_kernel: SOURCES required")
+      endif()
+      if(NOT ARG_LINKER_SCRIPT)
+          message(FATAL_ERROR "tinyos_create_kernel: LINKER_SCRIPT required")
+      endif()
+  
+      add_executable(kernel.elf ${ARG_SOURCES})
+  
+      if(ARG_LIBRARIES)
+          target_link_libraries(kernel.elf PRIVATE ${ARG_LIBRARIES})
+      endif()
+  
+      target_include_directories(kernel.elf
+          PRIVATE
+              ${CMAKE_CURRENT_SOURCE_DIR}/include
+              ${CMAKE_SOURCE_DIR}/kernel/include
+              ${CMAKE_BINARY_DIR}/include
+      )
+  
+      target_compile_options(kernel.elf PRIVATE ${TINYOS_COMMON_FLAGS})
+      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
+          target_compile_options(kernel.elf PRIVATE ${TINYOS_DEBUG_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
+          target_compile_options(kernel.elf PRIVATE ${TINYOS_RELEASE_FLAGS})
+      elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
+          target_compile_options(kernel.elf PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
+      endif()
+  
+      set_target_properties(kernel.elf PROPERTIES
+          LINK_FLAGS "-T ${ARG_LINKER_SCRIPT} -nostdlib -static"
+          LINK_DEPENDS "${ARG_LINKER_SCRIPT}"
+          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
+      )
+  
+      message(STATUS "  Created kernel executable: kernel.elf")
+      message(STATUS "    Linker script: ${ARG_LINKER_SCRIPT}")
+      message(STATUS "    Libraries: ${ARG_LIBRARIES}")
+  endfunction()
```

> TODO: Consider adding the create_lib  and create_module later, since they are currently not needed.

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
+#ifndef TINYOS_CONFIG_H
+#define TINYOS_CONFIG_H
+
+#define TINYOS_PROJECT_NAME     "@PROJECT_NAME@"
+#define TINYOS_VERSION_MAJOR    @PROJECT_VERSION_MAJOR@
+#define TINYOS_VERSION_MINOR    @PROJECT_VERSION_MINOR@
+#define TINYOS_VERSION_PATCH    @PROJECT_VERSION_PATCH@
+#define TINYOS_VERSION_STRING   "@PROJECT_VERSION@"
+
+#define TINYOS_BUILD_TYPE       "@CMAKE_BUILD_TYPE@"
+#define TINYOS_BUILD_DATE       "@TINYOS_BUILD_DATE@"
+#define TINYOS_BUILD_TIME       "@TINYOS_BUILD_TIME@"
+
+
+#endif 
```

This template uses CMake variables surrounded by `@` symbols. When you call `configure_file()`, CMake replaces them with the actual value.

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
+cmake_minimum_required(VERSION 3.20)
+project(TinyOS VERSION 0.1.0 LANGUAGES ASM_NASM C)
+
+set(CMAKE_C_STANDARD 17)
+set(CMAKE_C_STANDARD_REQUIRED ON)
+set(CMAKE_C_EXTENSIONS ON)  # Allow GNU extensions
+
+list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
+
+include(TinyOSPlatform)
+include(TinyOSHelpers)
+
+string(TIMESTAMP TINYOS_BUILD_DATE "%Y-%m-%d")
+string(TIMESTAMP TINYOS_BUILD_TIME "%H:%M:%S")
+
+add_subdirectory(kernel)
```

Let's also print the build configuration summary in the root `CMakeLists.txt`, to make it easier to spot any misconfigurations:

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

This will produce the `build/kernel.elf`file a bootable ELF executable.
But in order to actually boot it, we need to package it into a ISO image.

[!side]
ISO images are named after the ISO 9660 standard that specifies the file system for CD-ROMs.
[/!side]

## Build Flow

```
                    Source Files
                        │
        ┌───────────────┼───────────────┐
        │               │               │
        ▼               ▼               ▼
   multiboot.asm   boot.asm           main.c
        │               │               │
        ▼               ▼               ▼
      [NASM]           [NASM]          [Clang]
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
> ELF (Executable and Linkable Format) is the standard object file format on Unix-like systems (Linux, BSD, etc.).
> Like PE on Windows or Mach-O on macOS, it evolved from the assembler output format (.out) and later the common object file format (.coff).
> It's a container that holds:
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
