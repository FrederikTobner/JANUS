# The CMake Build System

We have source files. We have a linker script. But manually assembling and linking everything is tedious and error-prone. Time to automate.

> **The Crux: Why Not Just a Shell Script?**
>
> You could write a bash script: `nasm this, clang that, ld the other`. It would work! For about a week. Then you add a new source file and forget to update the script. Or you want debug builds vs. release builds. Or you work on macOS and your teammate uses Linux. Shell scripts don't scale.
>
> CMake handles dependency tracking, parallel builds, cross-platform differences, and more. It's the industry standard for good reason.

## Build System Architecture

TinyOS uses a modular CMake structure where each component is self-contained:

```
build system/
├── CMakeLists.txt (root)           # Orchestrates everything
├── cmake/
│   ├── TinyOSPlatform.cmake       # Compiler detection, flags
│   └── TinyOSHelpers.cmake        # Helper functions
├── boot/CMakeLists.txt            # Boot module
├── lib/*/CMakeLists.txt           # Library modules
├── arch/CMakeLists.txt            # Architecture layer
└── kernel/CMakeLists.txt          # Final kernel executable
```

**Why modular?** Each module builds independently. Change one file in `lib/buffer`, only that library rebuilds. The kernel just re-links. Fast incremental builds.

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

## Helper Functions

Create `cmake/TinyOSHelpers.cmake` with helper functions for kernel development:

```cmake
# cmake/TinyOSHelpers.cmake

# Create kernel executable with custom linker script
function(tinyos_create_kernel)
    cmake_parse_arguments(
        ARG
        ""
        "LINKER_SCRIPT"
        "SOURCES;LIBRARIES"
        ${ARGN}
    )
    
    # Create the kernel executable
    add_executable(kernel.elf ${ARG_SOURCES})
    
    # Link with provided libraries and objects
    if(ARG_LIBRARIES)
        target_link_libraries(kernel.elf PRIVATE ${ARG_LIBRARIES})
    endif()
    
    # Apply linker script
    if(ARG_LINKER_SCRIPT)
        target_link_options(kernel.elf PRIVATE
            -T ${ARG_LINKER_SCRIPT}
            -nostdlib
            -static
        )
        # Ensure rebuild when linker script changes
        set_target_properties(kernel.elf PROPERTIES
            LINK_DEPENDS ${ARG_LINKER_SCRIPT}
        )
    endif()
    
    # Set kernel-specific flags
    target_compile_options(kernel.elf PRIVATE
        -ffreestanding
        -fno-stack-protector
        -mno-red-zone
    )
    
    # Include directories
    target_include_directories(kernel.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/boot/include
    )
endfunction()
```

Key functions:

**`tinyos_create_kernel(SOURCES ... LIBRARIES ... LINKER_SCRIPT ...)`**

- Creates final `kernel.elf` executable
- Links all libraries and boot objects
- Applies custom linker script
- Sets entry point to `_start`

## Building the Kernel

First, create the root `CMakeLists.txt`:

```cmake
# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.20)
project(TinyOS ASM_NASM C)

# Include our helper functions
include(cmake/TinyOSHelpers.cmake)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Kernel compile flags
add_compile_options(
    -target x86_64-elf
    -ffreestanding
    -nostdlib
    -mno-red-zone
    -fno-stack-protector
    -Wall -Wextra
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/boot/include
)

# Build subdirectories
add_subdirectory(boot)
add_subdirectory(kernel)
```

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
NASM → multiboot.o, boot.o (boot module)
     ↓
Clang → *.o files (kernel, libraries)
     ↓
Linker (using linker.ld)
     ↓
kernel.elf (bootable!)
```

**Parallel builds:** Ninja automatically compiles modules in parallel. On a 4-core machine, this is 3-4× faster than sequential builds.

**Incremental builds:** Change one `.c` file, only that file and the final link step re-run. No wasted rebuilds.

## Verification

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

You should see segments loading at 0x100000 (1MB).

**Check for Multiboot2 header:**

```bash
readelf -S build/kernel.elf | grep multiboot
```

The `.multiboot` section should be present and properly aligned.

---

**Next: [The Kernel Entry Point →](kernel-entry.md)**
