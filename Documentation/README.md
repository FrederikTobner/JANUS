# JANUS Documentation

This directory contains the technical documentation for JANUS.

1. **[Setup.md](Setup.md)** - Install required dependencies (CMake, Clang, NASM, LLDB, QEMU)

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
  - Directory hierarchy
  - Module responsibilities and boundaries
  - Dependency graph and build order
  - CMake build system organization
  - Library structure

- **[Include-Hierarchy.md](Code-Organization/Global-Include-Hierarchy.md)**
  - Header file organization (public vs internal)
  - Include path structure
  - Forward declarations and reducing dependencies
  - Architecture-specific headers
  - CMake include directory configuration

- **[Global-Include-Hierarchy.md](Code-Organization/Global-Include-Hierarchy.md)**
  - What goes in `include/janus/`
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

## Contributing

When adding or modifying code:

1. **Review Core Principles** first to understand the project's philosophy
2. **Follow naming conventions** from Terminology.md
3. **Organize code** according to Module-Structure.md
4. **Design APIs** following API-Design.md guidelines
5. **Format code** using `.clang-format` (see Coding-Style.md)
6. **Document code** according to Documentation-Style.md
