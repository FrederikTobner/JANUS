# fmt — Formatted Output Library

Freestanding `printf`-style formatted output without `libc`. Consumers
supply a character-output callback; the library handles all format logic.

## Public API

```c
#include <fmt/output.h>

typedef void (*fmt_putc_fn)(char c, void * context);

// Write formatted output to an arbitrary sink.
s32 fmt_to(fmt_putc_fn putc, void * context, const char * format, ...);

// va_list variant.
s32 vfmt_to(fmt_putc_fn putc, void * context, const char * format, va_list args);
```

## Supported Specifiers

`%d` / `%i`, `%u`, `%x` / `%X`, `%o`, `%s`, `%c`, `%p`, `%%`.
Width and zero-padding (`%08x`) are supported. No floating point.

## Usage

```c
static void serial_putc(char c, void * ctx) { drivers_serial_putc(c); }

fmt_to(serial_putc, NULL, "boot: %s v%u.%u\n", name, major, minor);
```

## Files

- `include/fmt/output.h` — public API
- `src/output.c` — implementation
