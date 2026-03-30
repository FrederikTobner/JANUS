# Technical Philosophy

JANUS is guided by a small number of non-negotiable principles that shape every design decision, from directory layout to calling conventions.

## Educational Transparency

Every architectural choice should be understandable and well-documented. The codebase serves simultaneously as a working kernel and as teaching material. Where a design departs from the obvious approach, the reasoning is recorded — either in code comments, commit messages, or these documentation pages. References to processor manuals, specification documents, and relevant literature are included wherever they add clarity.

## Modular Architecture

JANUS is built as a collection of independent, well-defined modules, drawing on patterns from the Linux kernel and LLVM. Each module has clear boundaries, explicit dependencies, and can be understood in isolation. Circular dependencies between modules are forbidden and enforced by the build system.

This modularity extends to architecture-specific code: rather than centralising all platform logic in a single `arch/` tree, each subsystem and library contains its own `arch/` directory with the platform code it needs. The result is that a subsystem's entire implementation — generic and platform-specific — lives together.

## Public Structures Over Opaque Handles

Kernel data structures are defined publicly rather than hidden behind opaque pointer typedefs. In a freestanding kernel environment, the benefits of opaque handles (API stability across shared-library boundaries) do not apply, while their costs — pointer indirection, forced heap allocation, degraded cache locality, and poor debugger visibility — are severe.

Public structures enable stack allocation, structure embedding for cache-friendly layouts, and full visibility when inspecting state in LLDB or GDB. The rare exceptions are hardware abstraction boundaries where the underlying representation genuinely varies (e.g., framebuffer backends), and these are documented as such.
