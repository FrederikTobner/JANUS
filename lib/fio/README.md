# fio - Formatted I/O Library

Formatted input/output operations for kernel debugging and logging.

## Purpose

Provides printf-style formatted output for kernel code without depending on standard C library I/O functions.

## Planned Contents

### printf.h / printf.c
- `fio_printf()` - Formatted output to serial/VGA
- `fio_sprintf()` - Formatted string creation
- `fio_write_buffer()` - Buffer output
- `fio_debug_hex_dump()` - Memory dump for debugging

## Key Features

- **Format Specifiers**: Support for `%d`, `%x`, `%s`, `%p`, etc.
- **No libc Dependency**: Custom implementation for freestanding environment
- **Hardware Integration**: Direct output to serial port and VGA

## Dependencies

- `types` - Basic type definitions
- `buffer` - Character buffer operations

## Usage Example

```c
#include <lib/fio.h>

fio_printf("Kernel loaded at %p\n", kernel_base);
fio_printf("Page count: %d\n", page_count);
fio_debug_hex_dump(memory_region, 64);
```

## Status

⚠️ **Placeholder** - Implementation pending in future development phases.
