# display — Shared Display Type

Header-only library providing the `display_info_t` struct and `display_mode_t`
enum used to describe the display state the bootloader left the system in.

## Why a separate library?

Both the `boot` subsystem (which populates display info from bootloader
responses) and the `drivers` subsystem (which consumes it to configure the
TTY framebuffer renderer) need the same type. Placing it in `lib/display`
lets both subsystems depend on this library without depending on each other.

## Public API

```c
#include <display/display.h>

typedef enum {
    DISPLAY_MODE_NONE,         // No display provided
    DISPLAY_MODE_FRAMEBUFFER,  // Linear RGB framebuffer
    DISPLAY_MODE_VGA_TEXT,     // VGA text mode (x86_64 Multiboot2 only)
} display_mode_t;

typedef struct display_info {
    display_mode_t mode;
    void *         framebuffer;    // Base address (NULL for VGA text)
    u64            width;          // Horizontal resolution in pixels
    u64            height;         // Vertical resolution in pixels
    u64            pitch;          // Bytes per scanline
    u16            bpp;            // Bits per pixel
    u8             red_mask_shift;
    u8             green_mask_shift;
    u8             blue_mask_shift;
} display_info_t;
```

## Files

- `include/display/display.h` — the entire library (header-only, no sources)
