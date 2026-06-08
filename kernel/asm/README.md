# asm — Inline Assembly Primitives

This directory is the **only** permitted location for `__asm__ volatile` in the JANUS kernel C code.

It provides a thin layer of header-only, architecture-specific wrappers around privileged hardware instructions. Each wrapper is `static __always_inline`, so the compiler emits the instruction directly at every call site with no function-call overhead.

