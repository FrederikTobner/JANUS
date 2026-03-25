# Writing a VGA Text Buffer Driver

This tutorial implements a minimal VGA text mode driver. VGA text mode is the simplest way to put characters on screen from a bare-metal kernel — no framebuffer, no font renderer, just write bytes to a fixed memory address and they appear. By the end, the kernel will be able to clear the screen, print coloured text, and scroll.

## Prerequisites

A basic understanding of C and memory-mapped I/O. For hardware background, see [VGA Text Mode](../../wiki/graphics/vga-text-mode.md) and [Memory-Mapped I/O](../../wiki/io/mmio.md). The code assumes an x86_64 kernel running in long mode with the first 2 MiB identity-mapped (as established in the [mode switch tutorial](../boot/x86-64-mode-switch.md)). That identity mapping makes `0xB8000` directly accessible without additional paging work.

## The Memory Layout

The VGA text buffer lives at physical address `0xB8000`. The screen is an 80 × 25 grid of 16-bit cells:

| Bits | Content |
| ---- | ------- |
| 7–0 | ASCII character code |
| 11–8 | Foreground colour (0–15) |
| 15–12 | Background colour (0–15) |

The address of cell at column $x$, row $y$ is:

$$\text{address} = \texttt{0xB8000} + (y \times 80 + x) \times 2$$

Writing a 16-bit value to that address immediately updates the corresponding character on screen. No flush, no vsync — it is raw memory.

## Helpers

We start with two small helpers to build the 16-bit cell values:

```c
static inline u8 vga_entry_color(u8 foreground, u8 background) {
    return (u8)((foreground & 0x0F) | ((background & 0x0F) << 4));
}

static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16)((u16)uc | ((u16)color << 8));
}
```

`vga_entry_color` packs two 4-bit colour indices into a single attribute byte. `vga_entry` combines a character and an attribute into the 16-bit cell that the hardware expects.

## Cell Access Functions

With the entry-packing helpers in place, we can write two functions that read and write individual cells. Both perform bounds checking to prevent accidental writes outside the 4000-byte buffer — a stray index could silently corrupt whatever sits above `0xB8000 + 4000`:

```c
static inline void vga_write_cell(volatile u16 *buffer, u16 x, u16 y,
                                   char c, u8 fg, u8 bg) {
    if (buffer && x < VGA_TEXT_WIDTH && y < VGA_TEXT_HEIGHT) {
        buffer[y * VGA_TEXT_WIDTH + x] = vga_entry(c, vga_entry_color(fg, bg));
    }
}
```

The inverse operation reads a cell back, decomposing the 16-bit value into its constituent parts. Each output pointer is nullable, so callers can extract only the fields they need:

```c
static inline void vga_read_cell(volatile u16 *buffer, u16 x, u16 y,
                                  char *c, u8 *fg, u8 *bg) {
    if (buffer && x < VGA_TEXT_WIDTH && y < VGA_TEXT_HEIGHT) {
        u16 entry = buffer[y * VGA_TEXT_WIDTH + x];
        if (c)  *c  = (char)(entry & 0xFF);
        if (fg) *fg = (entry >> 8) & 0x0F;
        if (bg) *bg = (entry >> 12) & 0x0F;
    }
}
```

Reading cells back is useful for scrolling — we copy each character and its colours from row $n+1$ to row $n$ — and for debugging (inspecting what is currently on screen).

## Driver State

The driver tracks a cursor position and a current colour. The buffer pointer is declared `volatile` because the hardware reads from it independently of the CPU — the compiler must not optimise away or reorder writes.

```c
#include <janus/types.h>

#define VGA_TEXT_MEMORY 0xB8000
#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25

static volatile u16 *const vga_buffer = (volatile u16 *)VGA_TEXT_MEMORY;

static u16 vga_row    = 0;
static u16 vga_column = 0;
static u8  vga_color  = 0x07;   /* light grey on black */
```

## Initialisation and Clear

Initialisation resets the cursor and fills every cell with spaces. We also provide a separate clear function that preserves the current colour:

```c
void vga_text_init(void) {
    vga_row    = 0;
    vga_column = 0;
    vga_color  = vga_entry_color(7, 0);
    vga_text_clear();
}

void vga_text_clear(void) {
    for (u16 y = 0; y < VGA_TEXT_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[y * VGA_TEXT_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    vga_row    = 0;
    vga_column = 0;
}
```

## Scrolling

When the cursor moves past the last row, every row shifts up by one and the bottom row is cleared. This is a software-only operation — the VGA hardware has no built-in scroll:

```c
static void vga_text_scroll_if_needed(void) {
    if (vga_row < VGA_TEXT_HEIGHT)
        return;

    /* shift every row up by one */
    for (u16 y = 1; y < VGA_TEXT_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_TEXT_WIDTH + x] =
                vga_buffer[y * VGA_TEXT_WIDTH + x];
        }
    }

    /* blank the last row */
    for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
        vga_buffer[(VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH + x] =
            vga_entry(' ', vga_color);
    }

    vga_row = VGA_TEXT_HEIGHT - 1;
}
```

[!side]
This byte-by-byte copy could later be replaced with `memmove` for a slight speed improvement, but at 80 × 25 the difference is negligible.
[/!side]

Alternatively, if we lean on the `vga_read_cell` and `vga_write_cell` helpers from the previous section, the scroll becomes more readable and preserves per-cell colour information correctly:

```c
static void vga_text_scroll_if_needed(void) {
    if (vga_row < VGA_TEXT_HEIGHT)
        return;

    for (u16 y = 0; y < VGA_TEXT_HEIGHT - 1; y++) {
        for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
            char ch;
            u8 fg, bg;
            vga_read_cell(vga_buffer, x, y + 1, &ch, &fg, &bg);
            vga_write_cell(vga_buffer, x, y, ch, fg, bg);
        }
    }
    for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
        vga_write_cell(vga_buffer, x, VGA_TEXT_HEIGHT - 1, ' ',
                       vga_color & 0x0F, (vga_color >> 4) & 0x0F);
    }
    vga_row = VGA_TEXT_HEIGHT - 1;
}
```

This is the approach JANUS actually uses in its TTY driver — the shared scrolling logic reads and writes cells through an architecture contract, so the same code works for both VGA text mode and framebuffer backends.

## Writing Characters

`vga_text_putc` handles four control characters: newline (`\n`), carriage return (`\r`), tab (`\t`), and backspace (`\b`). Everything else is a printable character written at the current cursor position. Line wrapping and scrolling are checked after every character:

```c
void vga_text_putc(char c) {
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\t') {
        /* advance to the next 8-column tab stop */
        vga_column = (vga_column + 8) & ~7;
    } else if (c == '\b') {
        if (vga_column > 0)
            vga_column--;
    } else {
        vga_buffer[vga_row * VGA_TEXT_WIDTH + vga_column] =
            vga_entry((unsigned char)c, vga_color);
        vga_column++;
    }

    /* line wrap */
    if (vga_column >= VGA_TEXT_WIDTH) {
        vga_column = 0;
        vga_row++;
    }

    /* scroll if needed */
    vga_text_scroll_if_needed();
}

void vga_text_write_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_text_putc(str[i]);
    }
}
```

The tab-stop arithmetic `(col + 8) & ~7` rounds up to the next multiple of 8. Backspace moves the cursor left but does not erase the character — a full destructive backspace would additionally write a space at the new position. Line wrapping is automatic: when `vga_column` reaches 80, the cursor drops to the next row.

## Hardware Cursor

The VGA controller has a built-in blinking cursor, but we need to tell it where to put it. The cursor position is stored in two 8-bit registers (high byte and low byte) inside the CRT Controller (CRTC), accessed through I/O ports `0x3D4` (index) and `0x3D5` (data).

First, we need a helper to write to an x86 I/O port. This uses the `outb` instruction wrapped in inline assembly:

```c
static inline void outb(u16 port, u8 value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
```

With that in hand, updating the hardware cursor is four port writes — select the low-byte register (`0x0F`), write the low byte, select the high-byte register (`0x0E`), write the high byte:

```c
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5

void vga_set_cursor(u16 x, u16 y) {
    u16 pos = y * VGA_TEXT_WIDTH + x;
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (u8)(pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (u8)((pos >> 8) & 0xFF));
}
```

Call `vga_set_cursor(vga_column, vga_row)` at the end of `vga_text_putc` and `vga_text_clear` so the blinking underscore always tracks the write position. Without these calls the cursor stays at position zero regardless of where text is being printed.

[!side]
The CRTC registers are a vestige of the original IBM CGA adapter. Register `0x0E` holds the upper 8 bits of the cursor address and `0x0F` holds the lower 8 bits. The linear address is simply row × 80 + column.
[/!side]

## Changing Colours

The standard VGA palette provides 16 colours (0–15). Rather than remembering raw indices, we can define a named enumeration:

```c
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;
```

Indices 0–7 are the "dark" variants; 8–15 are their bright counterparts (the high bit acts as an intensity flag). Background colours traditionally use only the lower three bits (0–7), but on most modern hardware the blink bit (bit 3 of the background nibble) is repurposed as a bright-background flag, giving a full 16 × 16 colour matrix.

A convenience function lets the caller switch colours at any point — all subsequent characters use the new colour:

```c
void vga_text_set_color(vga_color_t foreground, vga_color_t background) {
    vga_color = vga_entry_color(foreground, background);
}
```

Common attribute values: `0x07` is light grey on black (the BIOS default), `0x0F` is white on black, `0x4F` is white on red — handy for error banners.

## Using the Driver

Initialise in `kernel_main` and print a test string:

```c
#include <drivers/vga_text.h>

/* inside kernel_main, after boot handoff validation: */
vga_text_init();
vga_text_write_string("hello world\n");
```

No QEMU flags are needed — VGA text mode is the default display. If you see `hello world` on the QEMU screen, the driver works end to end.

## How JANUS Layers This

The tutorial above presents a self-contained flat driver. In the actual JANUS kernel, VGA text mode is one backend behind a two-layer TTY abstraction:

1. **Shared logic** (`drivers/tty.h`, `src/tty.c`) — cursor tracking, scrolling, colour state, and the public API (`drivers_tty_putc`, `drivers_tty_clear`, `drivers_tty_set_cursor`, `drivers_tty_get_cursor`, `drivers_tty_get_size`, etc.). This layer is architecture-independent.
2. **Architecture contract** (`arch/drivers/tty.h`) — declares `arch_tty_init`, `arch_tty_write_cell`, `arch_tty_read_cell`, `arch_tty_set_cursor`, and `arch_tty_get_size`. Each architecture provides its own implementation.
3. **x86_64 backend** (`arch/x86_64/tty.c`) — implements the contract using either VGA text mode or a framebuffer renderer, selected at init time by inspecting the boot configuration. When the bootloader provides no framebuffer (Multiboot2 on BIOS), the driver defaults to VGA text mode at `0xB8000`. When a framebuffer is available (Limine on UEFI or BIOS), it uses a software glyph renderer instead.

The display mode is chosen during initialisation through a `tty_display_config_t` struct. If its `framebuffer` pointer is `NULL`, the x86_64 backend uses VGA text mode; otherwise it switches to framebuffer rendering. This decision is made once and the rest of the kernel never needs to know which backend is active.

This layering means the scrolling and cursor logic is written once and shared across architectures, while the hardware-specific bit packing (this tutorial) is confined to a single header (`vga.h`) on x86_64.

## Key Takeaways

- **VGA text mode is memory-mapped I/O.** Writing to `0xB8000` *is* drawing to the screen. No syscall, no driver stack.
- **The `volatile` qualifier is mandatory.** Without it, the compiler may coalesce or eliminate writes that have visible hardware side effects.
- **Bounds-check every access.** A column or row off by one writes past the 4000-byte buffer and silently corrupts unrelated memory.
- **Scrolling is a software concern.** The hardware only sees a flat 4000-byte buffer; the row-shift logic is entirely ours.
- **The hardware cursor needs explicit updates.** Four `outb` calls to the CRTC registers move the blinking underscore — without them it stays at position zero.
- **Handle all control characters.** Newline, carriage return, tab, and backspace each require special cursor movement; only printable characters produce a visible cell write.
- **This is x86-only.** ARM and RISC-V platforms use framebuffers or serial consoles instead. See [Framebuffers](../../wiki/graphics/framebuffers.md) for the platform-independent approach.
