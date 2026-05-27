# Framebuffers

A framebuffer is a contiguous region of memory whose contents are continuously scanned out to a display. 
Each cell in the buffer corresponds to a pixel on the screen, and writing a colour value to that cell causes the corresponding pixel to change the next time the display hardware refreshes.

## How It Works

The display controller reads from the framebuffer at a fixed refresh rate — typically 60 Hz — starting at the top-left corner and scanning left to right, top to bottom. 
Each pixel occupies a fixed number of bytes (commonly four: one each for red, green, blue, and either padding or an alpha channel). 
The address of pixel (x, y) in a linear framebuffer is:

$$\text{addr} = \text{base} + y \times \text{pitch} + x \times \text{bpp}$$

where *pitch* is the number of bytes per row (which may be larger than width × bpp due to alignment padding) and *bpp* is bytes per pixel.

## Pitch vs. Width

A common source of confusion is the distinction between the visible width and the pitch. 
The pitch includes any padding bytes the hardware or firmware inserts at the end of each row to satisfy alignment requirements. 
Code that assumes `pitch = width × bpp` will produce corrupted output on any display where the firmware adds padding. 
Always use the pitch reported by the bootloader or firmware, never calculate it from the width.

## Colour Formats

The most common format in modern boot environments is 32-bit BGRA: one byte each for blue, green, red, and alpha (or padding), stored in that order in memory. 
This is the native format for most UEFI framebuffers and the format the Limine boot protocol provides. 
Other formats (RGB, 16-bit, indexed colour) exist in older hardware and legacy VGA modes but are increasingly rare in boot-time framebuffers.

## Framebuffers in OS Development

During early kernel initialisation, the framebuffer is typically the only graphical output available. 
The bootloader requests a framebuffer from the firmware and passes its base address, dimensions, and pitch to the kernel. 
The kernel can then draw text by rasterising glyphs, bitmaps of character shapes, directly into the framebuffer memory.

This approach is straightforward but has significant limitations: there is no hardware cursor, no scrolling support, and no text-mode conveniences. 
The kernel must implement all of these in software. 
It is, however, portable, since it works identically on x86_64 and aarch64, provided the bootloader supplies a framebuffer.

[!side]
See [JANUS TTY driver](../../project/subsystems/drivers/tty.md) for how JANUS renders text into the boot framebuffer.
[/!side]
