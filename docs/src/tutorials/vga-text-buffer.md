# Writing a VGA Text Buffer Driver

This tutorial implements a minimal VGA text mode driver. VGA text mode is the simplest way to put characters on screen from a bare-metal kernel — no framebuffer, no font renderer, just write bytes to a fixed memory address and they appear. By the end, the kernel will be able to clear the screen, print coloured text, and scroll.

## Prerequisites

A basic understanding of C and memory-mapped I/O. For hardware background, see [VGA Text Mode](../concepts/hardware/vga-text-mode.md) and [Memory-Mapped I/O](../concepts/hardware/mmio.md). The code assumes an x86_64 kernel running in long mode with the first 2 MiB identity-mapped (as established in the [mode switch tutorial](x86-64-mode-switch.md)). That identity mapping makes `0xB8000` directly accessible without additional paging work.

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

[!margin]
This byte-by-byte copy could later be replaced with `memmove` for a slight speed improvement, but at 80 × 25 the difference is negligible.
[/!margin]

## Writing Characters

`vga_text_putc` handles newlines (`\n`) and carriage returns (`\r`) explicitly. Everything else is written at the current cursor position:

```c
void vga_text_putc(char c) {
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
        vga_text_scroll_if_needed();
        return;
    }

    if (c == '\r') {
        vga_column = 0;
        return;
    }

    vga_buffer[vga_row * VGA_TEXT_WIDTH + vga_column] =
        vga_entry((unsigned char)c, vga_color);

    vga_column++;
    if (vga_column >= VGA_TEXT_WIDTH) {
        vga_column = 0;
        vga_row++;
        vga_text_scroll_if_needed();
    }
}

void vga_text_write_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_text_putc(str[i]);
    }
}
```

Line wrapping is automatic: when `vga_column` reaches 80, the cursor drops to the next row.

## Changing Colours

The standard VGA palette provides 16 colours (0–15). A convenience function lets the caller switch colours at any point — all subsequent characters use the new colour:

```c
void vga_text_set_color(u8 foreground, u8 background) {
    vga_color = vga_entry_color(foreground, background);
}
```

Common colour indices: 0 = black, 1 = blue, 2 = green, 4 = red, 7 = light grey, 15 = white.

## Using the Driver

Initialise in `kernel_main` and print a test string:

```c
#include <drivers/vga_text.h>

/* inside kernel_main, after multiboot validation: */
vga_text_init();
vga_text_write_string("hello world\n");
```

No QEMU flags are needed — VGA text mode is the default display. If you see `hello world` on the QEMU screen, the driver works end to end.

## Key Takeaways

- **VGA text mode is memory-mapped I/O.** Writing to `0xB8000` *is* drawing to the screen. No syscall, no driver stack.
- **The `volatile` qualifier is mandatory.** Without it, the compiler may coalesce or eliminate writes that have visible hardware side effects.
- **Scrolling is a software concern.** The hardware only sees a flat 4000-byte buffer; the row-shift logic is entirely ours.
- **This is x86-only.** ARM and RISC-V platforms use framebuffers or serial consoles instead. See [Framebuffers](../concepts/hardware/framebuffers.md) for the platform-independent approach.
