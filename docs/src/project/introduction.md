# JANUS Project

This section documents everything specific to JANUS: the kernel architecture, subsystem design, developer tooling, and conventions for anyone working on or contributing to the project.

Because JANUS already ships Doxygen-annotated source code, the subsystem reference deliberately stays at the architectural level. It describes the high-level design, the module boundaries, and the data flow between subsystems, rather than duplicating the function-level documentation that lives in the code itself.

- **Architecture** covers the layer model, the module structure, and the three-tier include hierarchy that governs how subsystems expose their APIs.
- **Subsystems** documents each kernel subsystem and core service area — core, boot, memory management, kernel main, and drivers.
- **Tooling** provides practical cheat sheets for clang-tidy, Doxygen, LLDB, CMake, and QEMU.
- **Contributing** covers the coding style, naming conventions, and terminology used throughout the codebase.
- **Principles** documents the core technical philosophy and standards that guide design decisions.
- **Setup** walks through the required dependencies and build instructions for getting a development environment running.

For general OS concepts, see the [Wiki](../wiki/introduction.md) section.

