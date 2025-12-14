# TinyOS Module Structure

## Module Organization Philosophy

**Flat hierarchy. Independent modules. No spaghetti.**

TinyOS follows a modular structure inspired by Linux and LLVM: each major component is a first-class module with clear boundaries and **zero circular dependencies**. If you create a circular dependency, you've designed it wrong. Fix it.

## Top-Level Directory Structure

```
TinyOS/
├── kernel/           # Core kernel functionality
├── boot/             # Boot loader and initialization
├── arch/             # Architecture-specific code
├── lib/              # Utility libraries
├── mm/               # Memory management
├── drivers/          # Device drivers (future)
├── fs/               # File systems (future)
├── net/              # Network stack (future)
├── include/          # Global headers
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
│   └── bmunit/       # BMUnit testing framework
└── Documentation/    # Technical documentation
```

## Module Design Principles

### 1. Self-Contained Modules
Each module is an independent unit that:
- Builds as a static library (`.a` file)
- Has its own include directory
- Declares explicit dependencies
- Exports a clear public interface
- Can be tested independently

### 2. No Circular Dependencies

**Circular dependencies indicate incorrect abstraction layering.** If module A depends on B and B depends on A, the module boundaries are incorrectly designed.

Dependencies must flow in **one direction**:

```
           ┌──────────┐
           │  kernel  │
           └────┬─────┘
                │
       ┌────────┴────────┐
       ▼                 ▼
  ┌─────────┐      ┌─────────┐
  │   fio   │      │  arch   │
  └────┬────┘      └────┬────┘
       │                │
       └────────┬───────┘
                ▼
          ┌──────────┐
          │  buffer  │
          └──────────┘
```

If `buffer` needs something from `kernel`, you've layered it wrong. Move the functionality or redesign the interface.

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

### `lib/types/` - Basic Types and Compiler Attributes
Fundamental type definitions and compiler-specific macros.

**Structure:**
```
lib/types/
├── include/
│   └── lib/
│       ├── types.h      # Fixed-width types, size_t, bool
│       └── compiler.h   # Compiler attributes, macros
└── CMakeLists.txt
```

**Responsibilities:**
- Fixed-width integer types (uint8_t, int32_t, etc.)
- Size types (size_t, ssize_t, ptrdiff_t)
- Boolean type (bool, true, false)
- NULL definition
- Compiler attributes (__packed, __aligned, etc.)
- Common macros (likely, unlikely, ARRAY_SIZE)

### `lib/memory/` - Memory Manipulation
Low-level memory operations without libc.

**Structure:**
```
lib/memory/
├── memory.c           # Memory operations implementation
├── include/
│   └── lib/
│       └── memory.h   # Public interface
└── CMakeLists.txt
```

**Responsibilities:**
- memcpy, memmove, memset, memcmp
- memzero (explicit zero-fill)
- Raw memory region operations

### `lib/buffer/` - Character Buffer Operations
Character buffer manipulation, views, and operations.

**Structure:**
```
lib/buffer/
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

### `lib/fio/` - Formatted Input/Output
Formatted I/O, logging, and debug output.

**Structure:**
```
lib/fio/
├── fio.c              # Formatted I/O implementation
├── include/
│   └── lib/
│       └── fio.h      # Public interface
└── CMakeLists.txt
```

**Responsibilities:**
- Formatted output (printf-style)
- Debug output and hex dumps
- Stream abstractions
- Binary serialization

## Module Dependencies

### Dependency Graph (Phase 1)
```
kernel
├── depends on: fio, buffer, arch, boot
│
fio
├── depends on: buffer, types
│
arch
├── depends on: memory, types
│
buffer
├── depends on: memory, types
│
memory
└── depends on: types (header-only)
```

### CMake Dependency Declaration
```cmake
# lib/buffer/CMakeLists.txt
add_library(buffer STATIC buffer.c)
target_link_libraries(buffer PUBLIC
    memory
    types
)

# arch/CMakeLists.txt  
add_library(arch STATIC
    x86_64/io.c
    x86_64/serial.c
    x86_64/cpu.c
)
target_link_libraries(arch PUBLIC
    memory
    types
)

# kernel/CMakeLists.txt
add_executable(kernel main.c)
target_link_libraries(kernel
    fio
    buffer
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
│   ├── buffer.a       # Character buffer library
│   ├── fio.a          # I/O library
│   ├── memory.a       # Memory operations
│   └── types.a        # Basic types
├── arch/
│   └── libarch.a      # Architecture library
└── boot/
    └── libboot.a      # Boot components
```

### Final Kernel Linking
All libraries link into final kernel binary:
```
buffer.a + fio.a + arch.a + boot.a + kernel.o → kernel.bin
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
lib/buffer/
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

**Tests live WITH the code they test in dedicated test directories.**

TinyOS uses BMUnit (Bare Metal Unit), a KUnit-inspired testing framework designed for bare metal kernel development. The framework is located in `tools/bmunit/`. Tests are organized in separate `tests/` directories within each module and compiled conditionally.

### Test File Organization

Tests are organized in separate `tests/` subdirectories within each module:

```
lib/buffer/
├── buffer.c              # Implementation
├── include/
│   └── lib/
│       └── buffer.h
├── tests/                # Test directory
│   └── buffer_test.c     # Tests (compiled only if BUILD_TESTS=ON)
└── CMakeLists.txt

arch/x86_64/
├── io.c
├── serial.c
├── include/
│   └── arch/
│       ├── io.h
│       └── serial.h
├── tests/                # Test directory
│   ├── io_test.c         # I/O tests
│   └── serial_test.c     # Serial port tests
└── CMakeLists.txt

kernel/
├── main.c
├── panic.c
├── include/
│   └── kernel/
│       ├── kernel.h
│       └── panic.h
├── tests/                # Test directory
│   └── panic_test.c      # Panic handler tests
└── CMakeLists.txt
```

### Why Separate Test Directories?

**Benefits of dedicated test directories:**
- **Clean separation** - Source and tests physically separated
- **Easier GLOB patterns** - Build system can distinguish source from tests
- **Scalability** - Works well as test count grows
- **Clear intent** - Test directory signals testing infrastructure
- **Module ownership** - Tests still part of module, just organized
- **Conditional compilation** - Enable/disable per module

### CMake Test Integration

```cmake
# lib/buffer/CMakeLists.txt

# Always build the library
tinyos_add_library(buffer
    SOURCES
        buffer.c
)

# Conditionally build tests
if(BUILD_TESTS)
    tinyos_add_test(buffer_test
        SOURCES tests/buffer_test.c
        DEPENDS buffer
    )
endif()
```

# Conditionally build tests
if(BUILD_TESTS)
    tinyos_add_test(buffer_test
        SOURCES buffer_test.c
        DEPENDS buffer
    )
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