# VGA Text Mode

VGA text mode is the simplest way for a PC kernel to display characters on screen. It requires no driver initialisation, no mode setting, and no pixel-level rendering — the hardware maps an 80×25 grid of character cells directly to a region of physical memory. Write a byte into that region, and a character appears on screen.

## The Text Buffer

The buffer begins at physical address `0xB8000` and spans 4000 bytes (80 columns × 25 rows × 2 bytes per cell). Each cell is a 16-bit value laid out as:

| Bits  | Field       | Meaning                                                |
|-------|-------------|--------------------------------------------------------|
| 7–0   | Character   | ASCII code point                                       |
| 11–8  | Foreground  | Colour index (0–15)                                    |
| 14–12 | Background  | Colour index (0–7)                                     |
| 15    | Blink       | Blink enable (often repurposed as bright background)   |

The address of a cell at column $x$, row $y$ is:

$$\text{addr} = \texttt{0xB8000} + (y \times 80 + x) \times 2$$

Because the buffer sits in the first megabyte of physical memory, any kernel whose early page tables identity-map that range can write to the buffer immediately. No I/O port setup is required — the VGA controller watches this memory region and renders whatever it finds there.

## Colours

The standard VGA palette provides 16 foreground colours (0–15) and 8 background colours (0–7). The combined attribute byte is:

```c
uint8_t attribute = (background << 4) | foreground;
```

Common values: `0x07` is light grey on black (the default), `0x0F` is white on black, `0x4F` is white on red.

## Scrolling

VGA text mode provides no hardware scroll register in the way CGA did. Scrolling is implemented in software by copying each row one position upward (a `memmove` of 160 bytes per row) and clearing the last row. This is inexpensive on modern hardware but worth noting — the kernel must handle it explicitly.

## Limitations

VGA text mode is available only on x86. 
It is a legacy feature of the VGA standard and has no equivalent on aarch64 or other architectures. 
On modern UEFI systems, the firmware may not initialise a VGA text mode at all — the bootloader typically provides a graphical framebuffer instead, and the kernel renders text into pixel memory. 
JANUS supports both path but prefers the framebuffer when available, since it is more widely supported across architectures and firmware versions.
The vga text mode is only used as a fallback under x86_64 when no framebuffer is available.

[!side]
See [Framebuffers](framebuffers.md) for how pixel-based text rendering works, and [JANUS TTY Driver](../../project/subsystems/drivers/tty.md) for the driver that unifies both backends.
[/!side]
