# JANUS Driver Architecture

> **Status:** Proposal  
> **Date:** February 2, 2026

---

## Problem Statement

Generic driver interfaces with `init`, `read`, `write`, `shutdown` operations are:

1. **Not descriptive** — What does `write` mean for a framebuffer vs. a serial port?
2. **Not type-safe** — `void *buf` doesn't tell you what data format is expected
3. **Leaky abstraction** — Forces all drivers into the same mold

Runtime polymorphism via function pointers has drawbacks:

1. **Indirection overhead** — Every call requires loading a pointer from memory
2. **Cache/branch-predictor unfriendly** — Indirect calls are harder to optimize
3. **Unnecessary** — We compile for ONE architecture at a time

---

## Core Insight

**We don't need runtime polymorphism.**

The kernel is compiled for a single target architecture. The "polymorphism" happens at **build time** when CMake selects which `arch/` directory to compile. The linker then resolves symbols to the correct implementation.

This gives us **zero-overhead abstraction** — the same performance as if we had no abstraction at all.

---

## Design Principles

### 1. Direct Function Calls (Zero Overhead)

```c
// ❌ Runtime polymorphism: indirect call through pointer
struct serial_ops { void (*putc)(char c); };
const serial_ops_t *ops;
ops->putc('A');  // Load pointer from memory, then call

// ✅ Compile-time polymorphism: direct call
void arch_serial_putc(char c);  // Declared in contract header
arch_serial_putc('A');          // Direct call, resolved by linker
```

### 2. Three-Tier Header System (Existing Pattern)

```
Tier 1: Public API       →  drivers_serial_putc()   // Kernel code calls this
Tier 2: Arch Contract    →  arch_serial_putc()      // Declaration only
Tier 3: Arch Impl        →  arch/x86_64/serial.c    // Definition
```

The public API (Tier 1) can be a thin wrapper or just call the arch function directly:

```c
// Option A: Thin wrapper with validation
void drivers_serial_putc(char c) {
    if (g_serial_initialized) {
        arch_serial_putc(c);
    }
}

// Option B: Direct call (if arch handles uninitialized state)
#define drivers_serial_putc(c) arch_serial_putc(c)
```

### 3. Descriptive Function Names Per Driver Class

Each driver class has its own set of `arch_*` functions with **meaningful names**:

```c
// Serial driver contract
error_t arch_serial_init(u32 baud_rate);
void    arch_serial_putc(char c);
char    arch_serial_getc(void);
bool    arch_serial_tx_ready(void);
bool    arch_serial_rx_ready(void);

// Framebuffer driver contract  
error_t arch_fb_init(const fb_config_t *config);
void    arch_fb_put_pixel(u32 x, u32 y, u32 color);
void    arch_fb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void    arch_fb_copy_rect(u32 dx, u32 dy, u32 sx, u32 sy, u32 w, u32 h);

// Timer driver contract
error_t arch_timer_init(u32 frequency_hz);
u64     arch_timer_get_ticks(void);
void    arch_timer_delay_us(u32 microseconds);
```

---

## Directory Structure

```
kernel/subsys/drivers/
├── CMakeLists.txt
│
├── include/drivers/              # Tier 1: Public API
│   ├── serial.h                 # drivers_serial_* declarations
│   ├── framebuffer.h            # drivers_fb_* declarations
│   └── tty.h                    # drivers_tty_* declarations
│
├── serial.c                     # Public wrappers (optional validation)
├── framebuffer.c
├── tty.c                        # Generic logic (scrolling, cursor)
│
└── arch/
    ├── include/arch/drivers/    # Tier 2: Arch contracts
    │   ├── serial.h             # arch_serial_* declarations
    │   ├── framebuffer.h        # arch_fb_* declarations
    │   └── tty.h                # arch_tty_* declarations
    │
    ├── shared/                  # Shared implementations (optional)
    │   ├── pl011/               # PL011 UART (used by aarch64, some riscv)
    │   │   ├── pl011.c
    │   │   └── pl011.h
    │   └── ns16550/             # 16550 UART (used by x86, some riscv)
    │       ├── ns16550.c
    │       └── ns16550.h
    │
    ├── x86_64/
    │   ├── serial.c             # Implements arch_serial_*
    │   ├── framebuffer.c        # Implements arch_fb_*
    │   ├── tty.c                # Implements arch_tty_*
    │   └── include/arch/impl/drivers/
    │       └── io.h             # Port I/O: outb(), inb()
    │
    └── aarch64/
        ├── serial.c             # Implements arch_serial_*
        ├── framebuffer.c        # Implements arch_fb_*
        ├── tty.c                # Implements arch_tty_*
        └── include/arch/impl/drivers/
            └── mmio.h           # MMIO: mmio_read32(), mmio_write32()
```

---

## Interface Definitions

### Serial Driver

**Tier 1: Public API** (`include/drivers/serial.h`)

```c
#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <janus/types.h>

/// Initialize serial port at given baud rate
error_t drivers_serial_init(u32 baud_rate);

/// Send a single character (blocks until ready)
void drivers_serial_putc(char c);

/// Send a null-terminated string
void drivers_serial_puts(const char *str);

/// Receive a single character (blocks until available)
char drivers_serial_getc(void);

/// Check if transmitter can accept a character
bool drivers_serial_tx_ready(void);

/// Check if receiver has data available  
bool drivers_serial_rx_ready(void);

/// Check if serial is initialized and available
bool drivers_serial_available(void);

#endif
```

**Tier 2: Arch Contract** (`arch/include/arch/drivers/serial.h`)

```c
#ifndef ARCH_DRIVERS_SERIAL_H
#define ARCH_DRIVERS_SERIAL_H

#include <janus/types.h>

/**
 * Architecture must implement these functions.
 * Linker resolves to arch/$(ARCH)/serial.c
 */

/// Initialize hardware (configure baud rate, FIFOs, etc.)
error_t arch_serial_init(u32 baud_rate);

/// Transmit one character (may block waiting for TX ready)
void arch_serial_putc(char c);

/// Receive one character (may block waiting for RX ready)
char arch_serial_getc(void);

/// Non-blocking check: can we transmit?
bool arch_serial_tx_ready(void);

/// Non-blocking check: is data available?
bool arch_serial_rx_ready(void);

#endif
```

**Tier 3: x86_64 Implementation** (`arch/x86_64/serial.c`)

```c
#include <arch/drivers/serial.h>
#include <arch/impl/drivers/io.h>

#define COM1 0x3F8

error_t arch_serial_init(u32 baud_rate)
{
    u16 divisor = 115200 / baud_rate;
    
    outb(COM1 + 1, 0x00);           // Disable interrupts
    outb(COM1 + 3, 0x80);           // Enable DLAB
    outb(COM1 + 0, divisor & 0xFF); // Divisor low
    outb(COM1 + 1, divisor >> 8);   // Divisor high
    outb(COM1 + 3, 0x03);           // 8N1
    outb(COM1 + 2, 0xC7);           // Enable FIFO
    outb(COM1 + 4, 0x0B);           // RTS/DSR set
    
    // Loopback test
    outb(COM1 + 4, 0x1E);
    outb(COM1 + 0, 0xAE);
    if (inb(COM1) != 0xAE) return -1;
    outb(COM1 + 4, 0x0F);
    
    return 0;
}

bool arch_serial_tx_ready(void)
{
    return (inb(COM1 + 5) & 0x20) != 0;
}

void arch_serial_putc(char c)
{
    while (!arch_serial_tx_ready()) { }
    outb(COM1, (u8)c);
}

bool arch_serial_rx_ready(void)
{
    return (inb(COM1 + 5) & 0x01) != 0;
}

char arch_serial_getc(void)
{
    while (!arch_serial_rx_ready()) { }
    return (char)inb(COM1);
}
```

**Tier 3: aarch64 Implementation** (`arch/aarch64/serial.c`)

```c
#include <arch/drivers/serial.h>
#include <arch/impl/drivers/mmio.h>

// PL011 UART (QEMU virt machine)
#define UART_BASE 0x09000000
#define UART_DR   (UART_BASE + 0x00)
#define UART_FR   (UART_BASE + 0x18)

#define UART_FR_TXFF (1 << 5)  // TX FIFO full
#define UART_FR_RXFE (1 << 4)  // RX FIFO empty

error_t arch_serial_init(u32 baud_rate)
{
    (void)baud_rate;  // PL011 on QEMU doesn't need configuration
    return 0;
}

bool arch_serial_tx_ready(void)
{
    return (mmio_read32(UART_FR) & UART_FR_TXFF) == 0;
}

void arch_serial_putc(char c)
{
    while (!arch_serial_tx_ready()) { }
    mmio_write32(UART_DR, (u32)c);
}

bool arch_serial_rx_ready(void)
{
    return (mmio_read32(UART_FR) & UART_FR_RXFE) == 0;
}

char arch_serial_getc(void)
{
    while (!arch_serial_rx_ready()) { }
    return (char)mmio_read32(UART_DR);
}
```

**Generic Wrapper** (`serial.c`)

```c
#include <drivers/serial.h>
#include <arch/drivers/serial.h>

static bool g_initialized = false;

error_t drivers_serial_init(u32 baud_rate)
{
    error_t err = arch_serial_init(baud_rate);
    if (err == 0) g_initialized = true;
    return err;
}

void drivers_serial_putc(char c)
{
    if (g_initialized) arch_serial_putc(c);
}

void drivers_serial_puts(const char *str)
{
    while (*str) drivers_serial_putc(*str++);
}

char drivers_serial_getc(void)
{
    return g_initialized ? arch_serial_getc() : 0;
}

bool drivers_serial_tx_ready(void)
{
    return g_initialized && arch_serial_tx_ready();
}

bool drivers_serial_rx_ready(void)
{
    return g_initialized && arch_serial_rx_ready();
}

bool drivers_serial_available(void)
{
    return g_initialized;
}
```

---

### Framebuffer Driver

**Tier 2: Arch Contract** (`arch/include/arch/drivers/framebuffer.h`)

```c
#ifndef ARCH_DRIVERS_FRAMEBUFFER_H
#define ARCH_DRIVERS_FRAMEBUFFER_H

#include <janus/types.h>

typedef struct {
    void *base;
    u32 width;
    u32 height;
    u32 pitch;
    u8 bpp;
    u8 red_shift;
    u8 green_shift;
    u8 blue_shift;
} fb_config_t;

/// Initialize with bootloader-provided config
error_t arch_fb_init(const fb_config_t *config);

/// Draw a single pixel
void arch_fb_put_pixel(u32 x, u32 y, u32 color);

/// Fill rectangle with solid color
void arch_fb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);

/// Copy rectangle (for scrolling)
void arch_fb_copy_rect(u32 dst_x, u32 dst_y, 
                       u32 src_x, u32 src_y, 
                       u32 w, u32 h);

/// Get dimensions
void arch_fb_get_size(u32 *width, u32 *height);

#endif
```

The framebuffer implementation is largely architecture-independent (just memory writes), so it could live in `shared/` and be used by all architectures.

---

### Timer Driver

**Tier 2: Arch Contract** (`arch/include/arch/drivers/timer.h`)

```c
#ifndef ARCH_DRIVERS_TIMER_H
#define ARCH_DRIVERS_TIMER_H

#include <janus/types.h>

/// Initialize system timer at given frequency
error_t arch_timer_init(u32 frequency_hz);

/// Get ticks since boot
u64 arch_timer_get_ticks(void);

/// Get configured tick frequency
u32 arch_timer_get_frequency(void);

/// Busy-wait delay in microseconds
void arch_timer_delay_us(u32 us);

/// Register IRQ callback (optional)
typedef void (*timer_callback_t)(void);
void arch_timer_set_callback(timer_callback_t cb);

#endif
```

---

## Shared Code Pattern

When architectures share the same hardware (e.g., multiple platforms with PL011 UART):

**Recommended: Static Inline Helpers**

```c
// arch/shared/pl011/pl011.h
#ifndef SHARED_PL011_H
#define SHARED_PL011_H

#include <arch/impl/drivers/mmio.h>

#define PL011_DR   0x00
#define PL011_FR   0x18
#define PL011_FR_TXFF (1 << 5)
#define PL011_FR_RXFE (1 << 4)

static inline bool pl011_tx_ready(u64 base) {
    return (mmio_read32(base + PL011_FR) & PL011_FR_TXFF) == 0;
}

static inline void pl011_putc(u64 base, char c) {
    while (!pl011_tx_ready(base)) { }
    mmio_write32(base + PL011_DR, (u32)c);
}

static inline bool pl011_rx_ready(u64 base) {
    return (mmio_read32(base + PL011_FR) & PL011_FR_RXFE) == 0;
}

static inline char pl011_getc(u64 base) {
    while (!pl011_rx_ready(base)) { }
    return (char)mmio_read32(base + PL011_DR);
}

#endif
```

```c
// arch/aarch64/serial.c - Uses shared inline helpers
#include <arch/drivers/serial.h>
#include "../shared/pl011/pl011.h"

#define UART_BASE 0x09000000

error_t arch_serial_init(u32 baud_rate) {
    (void)baud_rate;
    return 0;
}

void arch_serial_putc(char c) {
    pl011_putc(UART_BASE, c);
}

char arch_serial_getc(void) {
    return pl011_getc(UART_BASE);
}

bool arch_serial_tx_ready(void) {
    return pl011_tx_ready(UART_BASE);
}

bool arch_serial_rx_ready(void) {
    return pl011_rx_ready(UART_BASE);
}
```

**Why static inline?**

1. **Zero overhead** — Compiler inlines the code, no function call
2. **Architecture still provides symbols** — Linker sees `arch_serial_putc`, not `pl011_putc`
3. **No global state in shared code** — Base address passed as parameter
4. **Easy to customize** — Architecture can add platform-specific logic around the helpers

---

## Build System

### Explicit Source Lists

Each architecture explicitly declares its sources:

```cmake
# arch/x86_64/Sources.cmake
set(ARCH_DRIVER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/serial.c
    ${CMAKE_CURRENT_LIST_DIR}/framebuffer.c
    ${CMAKE_CURRENT_LIST_DIR}/tty.c
)

set(ARCH_DRIVER_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include
)
```

```cmake
# arch/aarch64/Sources.cmake
set(ARCH_DRIVER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/serial.c
    ${CMAKE_CURRENT_LIST_DIR}/framebuffer.c
    ${CMAKE_CURRENT_LIST_DIR}/tty.c
)

set(ARCH_DRIVER_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/../shared/pl011  # Include shared headers
)
```

### Subsystem CMakeLists

```cmake
# drivers/CMakeLists.txt
include(${CMAKE_CURRENT_SOURCE_DIR}/arch/${JANUS_TARGET_ARCH}/Sources.cmake)

set(GENERIC_SOURCES
    serial.c
    framebuffer.c
    tty.c
)

janus_add_subsys(drivers 
    SOURCES 
        ${GENERIC_SOURCES}
        ${ARCH_DRIVER_SOURCES}
)

target_include_directories(drivers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/include
        ${ARCH_DRIVER_INCLUDES}
)
```

---

## Summary

| Aspect | Approach |
|--------|----------|
| **Abstraction** | Compile-time (link-time symbol resolution) |
| **Overhead** | Zero — direct function calls |
| **Contract** | `arch_*` function declarations in Tier 2 headers |
| **Implementation** | One `arch/<arch>/*.c` per driver per architecture |
| **Shared code** | Static inline helpers in `arch/shared/` |
| **Build** | Explicit `Sources.cmake` per architecture |

### Call Chain Example

```
kernel_main()
    → drivers_serial_puts("Hello")      // Tier 1: generic wrapper
        → drivers_serial_putc('H')
            → arch_serial_putc('H')     // Tier 2: direct call
                → outb(COM1, 'H')       // Tier 3: x86 implementation
```

At the machine code level, this compiles down to:

```asm
; With LTO enabled, the entire chain collapses to:
mov dx, 0x3F8
mov al, 'H'
out dx, al
```

**No vtables. No function pointers. No indirection. Zero overhead.**

---

## Comparison: This vs. Your Current Code

Your existing code already follows this pattern! For example:

```c
// tty.c (generic)
#include <arch/drivers/tty.h>
arch_tty_write_cell(x, y, ch, fg, bg);  // Direct call

// arch/x86_64/tty.c
void arch_tty_write_cell(...) { /* VGA or framebuffer */ }
```

This document formalizes the pattern and extends it to other driver classes (serial, timer, etc.) with more descriptive function names.
