# TinyOS Kernel Module

Core kernel functionality including initialization, panic handling, and main kernel loop.

## Contents

- `main.c` - Kernel entry point (to be created in T6)
- `panic.c` - Kernel panic and error handling
- `init.c` - Kernel initialization sequence

## Public Interface

Headers in `include/kernel/`:
- `kernel.h` - Main kernel definitions
- `panic.h` - Panic handling functions
- `init.h` - Initialization functions

## Dependencies

- types (type definitions)
- memory (memory operations)
- fio (I/O operations)
- arch (architecture layer)
- boot (boot components)
