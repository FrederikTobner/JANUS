# TinyOS

An educational x86-64 operating system kernel written in C17 and assembly, focusing on low-level hardware interaction and modular architecture.

## Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd TinyOS

# Build the kernel
cmake -B build
cmake --build build

# Run in QEMU
qemu-system-x86_64 -kernel build/kernel.elf -serial stdio
```

## Project Structure

```
TinyOS/
├── kernel/           # Core kernel functionality
├── boot/             # Multiboot2 boot loader
├── arch/             # Architecture-specific code (x86_64)
├── lib/              # Kernel libraries (libkbuffer, libkio, libkstd)
├── bmunit/           # BMUnit testing framework
├── memoryman/        # Memory management
├── drivers/          # Device drivers
├── include/          # Global headers
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
└── Documentation/    # Technical documentation
```

## Technology Stack

- **Language**: C17 (ISO/IEC 9899:2018) with GNU extensions
- **Compiler**: Clang 17
- **Debugger**: LLDB 17
- **Assembler**: NASM (Intel syntax)
- **Build System**: CMake 3.20+
- **Target**: x86_64-elf (bare metal)
- **Testing**: BMUnit (Bare Metal Unit testing framework)

## Documentation

Comprehensive documentation is in the [`Documentation/`](Documentation/) folder:

### Core Principles
- [Philosophy.md](Documentation/Core-Principles/Philosophy.md) - Technical philosophy and design values
- [Terminology.md](Documentation/Core-Principles/Terminology.md) - Naming conventions
- [Standards.md](Documentation/Core-Principles/Standards.md) - C17 standards and compiler requirements

### Code Organization
- [Module-Structure.md](Documentation/Code-Organization/Module-Structure.md) - Module hierarchy and dependencies
- [API-Design.md](Documentation/Code-Organization/API-Design.md) - Function design and naming patterns
- [Include-Hierarchy.md](Documentation/Code-Organization/Include-Hierarchy.md) - Header organization

### Implementation
- [Coding-Style.md](Documentation/Implementation/Coding-Style.md) - Code style guide
- [Hardware-Abstraction.md](Documentation/Implementation/Hardware-Abstraction.md) - Hardware interaction patterns
- [Documentation-Style.md](Documentation/Implementation/Documentation-Style.md) - Comment guidelines
- [BMUnit-Testing.md](Documentation/Implementation/BMUnit-Testing.md) - Testing framework and best practices

## Testing

TinyOS uses **BMUnit** (Bare Metal Unit), a testing framework inspired by Linux kernel's KUnit but adapted for bare metal development.

Tests are embedded within modules (not in a separate `tests/` folder):

```
lib/libkbuffer/
├── buffer.c              # Implementation
├── buffer_test.c         # Tests
└── include/lib/buffer.h
```

**Build and run tests:**

```bash
# Build with tests enabled
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run test kernel
qemu-system-x86_64 -kernel build/tinyos_test.elf -serial stdio -display none
```

See [BMUnit-Testing.md](Documentation/Implementation/BMUnit-Testing.md) for complete testing documentation.

## Development Principles

- **Hardware-First**: Direct hardware interaction, no unnecessary abstractions
- **Modular Architecture**: Independent modules with zero circular dependencies
- **Explicit Over Implicit**: Clear, verbose code over clever abstractions
- **Zero Warnings**: All code compiles cleanly with `-Wall -Wextra -Wpedantic`
- **Exposed Structures**: No opaque handles (Linux kernel philosophy)
- **Educational Focus**: Code as both implementation and teaching material

## Building

### Prerequisites

- Clang 17+
- NASM
- CMake 3.20+
- QEMU (for testing)

### Build Commands

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build
```

## Running

```bash
# Run in QEMU with serial output
qemu-system-x86_64 -kernel build/kernel.elf -serial stdio

# Run with debugging
qemu-system-x86_64 -kernel build/kernel.elf -s -S -serial stdio
# In another terminal:
lldb build/kernel.elf -o "gdb-remote localhost:1234"
```

## License

[License information to be added]

## Contributing

[Contributing guidelines to be added]
