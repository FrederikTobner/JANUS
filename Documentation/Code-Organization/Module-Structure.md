# TinyOS Module Structure

## Module Organization Philosophy

> "Controlling complexity is the essence of computer programming."
> — *Brian Kernighan*

**Flat hierarchy. Independent modules. No spaghetti.**

TinyOS follows a modular structure inspired by Linux and LLVM: each major component is a first-class module with clear boundaries and **zero circular dependencies**. If you create a circular dependency, you've designed it wrong. Fix it.

## Top-Level Directory Structure

```
TinyOS/
├── kernel/           # Core kernel functionality
├── boot/             # Boot loader and initialization
├── arch/             # Architecture-specific code
├── lib/              # Utility libraries
├── bmunit/           # BMUnit testing framework
├── memoryman/        # Memory management
├── drivers/          # Device drivers (future)
├── fs/               # File systems (future)
├── net/              # Network stack (future)
├── include/          # Global headers
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
└── Documentation/    # Technical documentation
```

## Module Design Principles

> "The Linux philosophy is 'Laugh in the face of danger'. Oops. Wrong One. 'Do it yourself'. Yes, that's it."
> — *Linus Torvalds*

### 1. Self-Contained Modules
Each module is an independent unit that:
- Builds as a static library (`.a` file)
- Has its own include directory
- Declares explicit dependencies
- Exports a clear public interface
- Can be tested independently

### 2. No Circular Dependencies

**Circular dependencies are bugs.** If module A depends on B and B depends on A, you've fucked up your abstraction layers.

Dependencies go **one direction**:

```
           ┌──────────┐
           │  kernel  │
           └────┬─────┘
                │
       ┌────────┴────────┐
       ▼                 ▼
  ┌─────────┐      ┌─────────┐
  │ libkio  │      │  arch   │
  └────┬────┘      └────┬────┘
       │                │
       └────────┬───────┘
                ▼
          ┌──────────┐
          │ libkbuffer│
          └──────────┘
```

If `libkbuffer` needs something from `kernel`, you've layered it wrong. Move the functionality or redesign the interface.

### 3. Clear Layering
```
┌─────────────────────────────────────────┐
│  Application Layer (future)             │
├─────────────────────────────────────────┤
│  Kernel Layer (kernel/)                 │
├─────────────────────────────────────────┤
│  Subsystem Layer (mm/, drivers/, etc.)  │
├─────────────────────────────────────────┤
│  Library Layer (lib/*)                  │
├─────────────────────────────────────────┤
│  Architecture Layer (arch/x86_64/)      │
├─────────────────────────────────────────┤
│  Boot Layer (boot/)                     │
└─────────────────────────────────────────┘
```

## Core Modules

### `kernel/` - Core Kernel
Main kernel functionality and entry point.

**Structure:**
```
kernel/
├── main.c              # Kernel entry point
├── panic.c             # Panic handling
├── init.c              # Kernel initialization
├── include/
│   └── kernel/
│       ├── kernel.h    # Main kernel interface
│       ├── panic.h     # Panic functions
│       └── init.h      # Initialization
├── linker.ld           # Memory layout
└── CMakeLists.txt      # Build configuration
```

**Responsibilities:**
- Kernel initialization sequence
- Main kernel loop
- Panic and error handling
- Global kernel state

### `boot/` - Boot Module
Boot loader interface and early initialization.

**Structure:**
```
boot/
├── multiboot.asm       # Multiboot2 header
├── boot.asm            # Boot entry point
├── include/
│   └── boot/
│       └── multiboot.h # Multiboot definitions
└── CMakeLists.txt
```

**Responsibilities:**
- Multiboot2 header and compliance
- Initial CPU state setup (stack, registers)
- Transition from boot loader to kernel

### `arch/` - Architecture Abstraction
Architecture-specific code with generic interfaces.

**Structure:**
```
arch/
├── x86_64/            # x86-64 implementation
│   ├── io.c           # Port I/O
│   ├── serial.c       # Serial port driver
│   ├── vga.c          # VGA text mode
│   ├── cpu.c          # CPU-specific operations
│   └── include/
│       └── arch/
│           ├── io.h
│           ├── serial.h
│           ├── vga.h
│           └── cpu.h
├── include/
│   └── arch/
│       └── types.h    # Arch-agnostic types
└── CMakeLists.txt
```

**Responsibilities:**
- Hardware I/O operations
- CPU-specific functions
- Platform initialization
- Hardware abstraction interfaces

## Library Modules

### `lib/libkbuffer/` - Character Buffer Operations
Character buffer manipulation, views, and operations.

**Structure:**
```
lib/libkbuffer/
├── buffer.c           # Buffer implementation
├── include/
│   └── lib/
│       └── buffer.h   # Public interface
└── CMakeLists.txt
```

**Responsibilities:**
- Character buffer creation and destruction
- Buffer manipulation (append, insert, delete)
- Buffer views and slicing
- Memory-safe buffer operations

### `lib/libkio/` - Input/Output Operations
Formatted I/O, logging, and debug output.

**Structure:**
```
lib/libkio/
├── io.c               # I/O implementation
├── include/
│   └── lib/
│       └── io.h       # Public interface
└── CMakeLists.txt
```

**Responsibilities:**
- Formatted output (printf-style)
- Debug output and hex dumps
- Stream abstractions
- Binary serialization

### `lib/libkstd/` - Standard Library Subset
Essential standard library functionality for kernel use.

**Structure:**
```
lib/libkstd/
├── stdint.c
├── stddef.c
├── include/
│   └── lib/
│       ├── stdint.h
│       └── stddef.h
└── CMakeLists.txt
```

**Responsibilities:**
- Basic type definitions
- Essential macros and constants
- Compiler intrinsics wrappers

## Module Dependencies

### Dependency Graph (Phase 1)
```
kernel
├── depends on: libkio, libkbuffer, arch, boot
│
libkio
├── depends on: libkbuffer, arch
│
arch
├── depends on: libkbuffer
│
libkbuffer
└── depends on: libkstd (minimal)
```

### CMake Dependency Declaration
```cmake
# lib/libkbuffer/CMakeLists.txt
add_kernel_library(kbuffer
    SOURCES buffer.c
    DEPENDS kstd
)

# arch/CMakeLists.txt  
add_kernel_library(arch
    SOURCES 
        x86_64/io.c
        x86_64/serial.c
        x86_64/cpu.c
    DEPENDS kbuffer
)

# kernel/CMakeLists.txt
add_executable(kernel main.c)
target_link_libraries(kernel
    kio
    kbuffer
    arch
    boot
)
```

## Module Build System

### Static Library Generation
Each module builds as a static library:
```
build/
├── lib/
│   ├── libkbuffer.a   # Character buffer library
│   ├── libkio.a       # I/O library
│   └── libkstd.a      # Standard library subset
├── arch/
│   └── libarch.a      # Architecture library
└── boot/
    └── libboot.a      # Boot components
```

### Final Kernel Linking
All libraries link into final kernel binary:
```
libkbuffer.a + libkio.a + libarch.a + boot.a + kernel.o → kernel.bin
```

## Module Interfaces

### Public vs Private Headers

**Public headers** (in `include/` directories):
- Exported to other modules
- Define module's public API
- Installed to global include path

**Private headers** (in module root or subdirs):
- Internal to module only
- Not visible to other modules
- Implementation details

Example:
```
lib/libkbuffer/
├── buffer.c              # Implementation
├── buffer_internal.h     # Private header (implementation details)
└── include/
    └── lib/
        └── buffer.h      # Public header (API)
```

## Future Module Expansion

### Memory Management Module
```
mm/
├── page_frame.c       # Physical memory management
├── vmm.c              # Virtual memory management
├── kmalloc.c          # Kernel heap allocator
└── include/
    └── mm/
        ├── page_frame.h
        ├── vmm.h
        └── kmalloc.h
```

### Device Driver Module
```
drivers/
├── keyboard/
│   ├── ps2_keyboard.c
│   └── include/
├── storage/
│   ├── ata.c
│   └── include/
└── CMakeLists.txt
```

### File System Module
```
fs/
├── vfs/               # Virtual file system
├── ext2/              # ext2 implementation
├── fat32/             # FAT32 implementation (future)
└── include/
```

## Module Documentation Requirements

Each module must have:
1. **README.md** - Module purpose and overview
2. **API documentation** - In public headers
3. **Implementation notes** - Complex algorithms explained
4. **Dependency documentation** - What it needs and why

## Module Testing

### BMUnit: Embedded Testing

**Tests live WITH the code they test, not in a separate directory.**

TinyOS uses BMUnit (Bare Metal Unit), a KUnit-inspired testing framework designed for bare metal kernel development. Tests are embedded within modules and compiled conditionally.

### Test File Organization

```
lib/libkbuffer/
├── buffer.c              # Implementation
├── buffer_test.c         # Tests (compiled only if BUILD_TESTS=ON)
├── include/
│   └── lib/
│       └── buffer.h
└── CMakeLists.txt

arch/x86_64/
├── io.c
├── io_test.c             # I/O tests
├── serial.c
├── serial_test.c         # Serial port tests
└── CMakeLists.txt

kernel/
├── main.c
├── panic.c
├── panic_test.c          # Panic handler tests
└── CMakeLists.txt
```

### Why Embedded Tests?

**Following Linux kernel convention:**
- **Locality** - Tests right next to code being tested
- **Module ownership** - Maintainer owns both code and tests
- **No artificial separation** - Tests are part of the module
- **Conditional compilation** - Enable/disable per module

**No separate `tests/` folder.** That's a userspace pattern that doesn't fit kernel development.

### CMake Test Integration

```cmake
# lib/libkbuffer/CMakeLists.txt

# Always build the library
add_library(kbuffer STATIC buffer.c)

# Conditionally build tests
if(BUILD_TESTS)
    add_library(kbuffer_test STATIC
        buffer_test.c
    )
    
    target_link_libraries(kbuffer_test
        kbuffer
        bmunit
    )
    
    set_property(GLOBAL APPEND PROPERTY BMUNIT_TEST_MODULES kbuffer_test)
endif()
```

### Running Tests

```bash
# Build with tests enabled
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run test kernel in QEMU
qemu-system-x86_64 -kernel build/tinyos_test.elf -serial stdio
```

**See [BMUnit-Testing.md](../Implementation/BMUnit-Testing.md) for complete testing documentation.**