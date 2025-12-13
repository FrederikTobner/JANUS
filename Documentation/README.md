# TinyOS Documentation

**Read this before writing code.**

This directory contains the technical documentation for TinyOS. It's not optional reading - it defines how we write code, name things, and organize the project.

## Quick Start

**New to the project? Read these in order:**

1. [Core-Principles/Philosophy.md](Core-Principles/Philosophy.md) - Why we do things this way
2. [Core-Principles/Terminology.md](Core-Principles/Terminology.md) - What words to use (and not use)
3. [Code-Organization/Module-Structure.md](Code-Organization/Module-Structure.md) - Where code goes
4. [Implementation/Coding-Style.md](Implementation/Coding-Style.md) - How to format it

**Don't skip these. They'll save you time in code review.**

## Documentation Structure

The documentation is organized into three main categories:

### Core Principles

Foundational documents that define the project's philosophy and standards.

- **[Philosophy.md](Core-Principles/Philosophy.md)**
  - Mission statement and project goals
  - Core values (simplicity, clarity, correctness)
  - Design principles (modularity, hardware awareness)
  - Quality standards and testing requirements

- **[Terminology.md](Core-Principles/Terminology.md)**
  - Low-level naming conventions
  - Preferred technical terms (hashtable vs map, character buffer vs string)
  - Type naming (\_t suffix, descriptive names)
  - Constant and macro naming (SCREAMING_CASE)

- **[Standards.md](Core-Principles/Standards.md)**
  - C17 language standard compliance
  - Compiler requirements (Clang 17+)
  - Type system (fixed-width integers, bool)
  - Error handling patterns (return codes, errno)
  - Platform assumptions (x86-64, freestanding)

### Code Organization

Documents describing the architecture and structure of the codebase.

- **[Module-Structure.md](Code-Organization/Module-Structure.md)**
  - Directory hierarchy (kernel/, boot/, arch/, lib/)
  - Module responsibilities and boundaries
  - Dependency graph and build order
  - CMake build system organization
  - Library structure (types, memory, buffer, fio)

- **[API-Design.md](Code-Organization/API-Design.md)**
  - Function naming patterns (module_action_object)
  - Parameter conventions (outputs last, counts follow buffers)
  - Return value standards (success/error codes)
  - Error handling (negative error codes, NULL returns)
  - Opaque types and handle patterns

- **[Include-Hierarchy.md](Code-Organization/Include-Hierarchy.md)**
  - Header file organization (public vs internal)
  - Include path structure
  - Forward declarations and reducing dependencies
  - Architecture-specific headers
  - CMake include directory configuration

- **[Global-Include-Hierarchy.md](Code-Organization/Global-Include-Hierarchy.md)**
  - What goes in `include/tinyos/`
  - Global vs module-specific headers
  - When to promote headers to global
  - Include patterns and examples
  - Migration policy for cross-cutting headers

### Implementation

Practical guides for writing code that adheres to project standards.

- **[Coding-Style.md](Implementation/Coding-Style.md)**
  - `.clang-format` configuration explained
  - Indentation and spacing (4 spaces, 120 columns)
  - Brace style (Linux kernel style)
  - Pointer alignment (middle)
  - Type qualifiers (right-side)
  - Control structures and switch statements
  - Macro alignment
  - Complete formatting examples

- **[Hardware-Abstraction.md](Implementation/Hardware-Abstraction.md)**
  - Memory-mapped I/O (MMIO) patterns
  - Port I/O (x86-64 inb/outb)
  - Bit manipulation macros
  - CPU control (control registers, MSRs, CPUID)
  - Interrupt control (cli/sti, critical sections)
  - Memory barriers and cache control
  - Atomic operations and spinlocks
  - Device driver patterns

- **[Documentation-Style.md](Implementation/Documentation-Style.md)**
  - Comment principles (explain "why", not "what")
  - Function documentation format (Doxygen-style)
  - Inline comments and section markers
  - File headers
  - Structure and type documentation
  - Algorithm documentation
  - Hardware-specific documentation
  - TODO/FIXME conventions
  - Technical writing style

- **[BMUnit-Testing.md](Implementation/BMUnit-Testing.md)**
  - BMUnit testing framework overview
  - Test organization (embedded tests, not separate folder)
  - Assertion macros (EXPECT vs ASSERT)
  - Test suite structure and registration
  - CMake test integration
  - Running tests in QEMU
  - Differences from KUnit
  - Testing best practices

## Document Purpose Summary

| Document | Purpose | Audience |
|----------|---------|----------|
| Philosophy.md | Define project mission and values | All contributors |
| Terminology.md | Establish naming vocabulary | All contributors |
| Standards.md | Specify language and platform requirements | All contributors |
| Module-Structure.md | Describe codebase organization | Developers, new contributors |
| API-Design.md | Guide interface design decisions | Library authors, API designers |
| Include-Hierarchy.md | Organize header dependencies | All developers |
| Global-Include-Hierarchy.md | Define global include directory | All developers |
| Coding-Style.md | Enforce consistent formatting | All developers |
| Hardware-Abstraction.md | Guide low-level hardware code | Kernel developers, driver authors |
| Documentation-Style.md | Guide comment and documentation writing | All contributors |
| BMUnit-Testing.md | Testing framework and practices | All developers |

## Contributing

When adding or modifying code:

1. **Review Core Principles** first to understand the project's philosophy
2. **Follow naming conventions** from Terminology.md
3. **Organize code** according to Module-Structure.md
4. **Design APIs** following API-Design.md guidelines
5. **Format code** using `.clang-format` (see Coding-Style.md)
6. **Document code** according to Documentation-Style.md

## Finding Information

### By Topic

**Architecture and Design:**
- Module boundaries → [Module-Structure.md](Code-Organization/Module-Structure.md)
- API contracts → [API-Design.md](Code-Organization/API-Design.md)
- Header organization → [Include-Hierarchy.md](Code-Organization/Include-Hierarchy.md)

**Coding Standards:**
- Formatting rules → [Coding-Style.md](Implementation/Coding-Style.md)
- Naming conventions → [Terminology.md](Core-Principles/Terminology.md)
- Language features → [Standards.md](Core-Principles/Standards.md)

**Low-Level Programming:**
- Hardware access → [Hardware-Abstraction.md](Implementation/Hardware-Abstraction.md)
- Register definitions → [Hardware-Abstraction.md](Implementation/Hardware-Abstraction.md)
- Atomic operations → [Hardware-Abstraction.md](Implementation/Hardware-Abstraction.md)

**Documentation:**
- Comment style → [Documentation-Style.md](Implementation/Documentation-Style.md)
- Function headers → [Documentation-Style.md](Implementation/Documentation-Style.md)
- Technical writing → [Documentation-Style.md](Implementation/Documentation-Style.md)

**Testing:**
- Test framework → [BMUnit-Testing.md](Implementation/BMUnit-Testing.md)
- Test organization → [Module-Structure.md](Code-Organization/Module-Structure.md#module-testing)
- Writing tests → [BMUnit-Testing.md](Implementation/BMUnit-Testing.md)

### Common Questions

**Q: How should I name my functions?**
A: See [Terminology.md](Core-Principles/Terminology.md) for naming conventions and [API-Design.md](Code-Organization/API-Design.md) for function naming patterns.

**Q: What coding style does the project use?**
A: Linux kernel brace style with 4-space indentation, 120-column limit. See [Coding-Style.md](Implementation/Coding-Style.md) for complete details.

**Q: How do I organize a new module?**
A: Follow the structure in [Module-Structure.md](Code-Organization/Module-Structure.md). Each module is a separate static library with public/internal headers.

**Q: How do I access hardware registers?**
A: Use the MMIO and port I/O patterns from [Hardware-Abstraction.md](Implementation/Hardware-Abstraction.md).

**Q: What should I document in function comments?**
A: Document the purpose, parameters, return values, error conditions, and preconditions. See [Documentation-Style.md](Implementation/Documentation-Style.md).

**Q: Can I use C++ features?**
A: No. TinyOS is written in C17 only. See [Standards.md](Core-Principles/Standards.md) for language requirements.

**Q: What types should I use for integers?**
A: Use fixed-width types from `stdint.h` (`uint8_t`, `uint32_t`, etc.). See [Standards.md](Core-Principles/Standards.md).

**Q: How do I write tests?**
A: Tests live with the code they test (e.g., `buffer_test.c` next to `buffer.c`). Use BMUnit framework. See [BMUnit-Testing.md](Implementation/BMUnit-Testing.md).

**Q: Why isn't there a `tests/` folder?**
A: Tests are embedded in modules, following Linux kernel convention. See [Module-Structure.md](Code-Organization/Module-Structure.md#module-testing).

## Document Maintenance

This documentation is version-controlled and should be updated when:

- New design patterns are established
- Coding standards change
- Module structure evolves
- New architectural decisions are made

When updating documentation:

1. Keep examples consistent with current codebase
2. Update cross-references if document structure changes
3. Maintain the table of contents in this README
4. Review related documents for consistency

## External References

Key external specifications referenced in the documentation:

- **Intel SDM**: [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- **Multiboot2**: [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
- **C17 Standard**: ISO/IEC 9899:2018
- **LLVM**: [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html) (inspiration, not requirement)
- **Linux Kernel**: [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html) (brace style reference)

## License

TinyOS documentation is licensed under the same terms as the project code.

## Feedback

If you find documentation that is:
- Unclear or ambiguous
- Outdated or incorrect
- Missing important information

Please open an issue or submit a pull request with improvements.
