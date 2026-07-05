# gfx — kernel software-graphics library

Stateless pixel primitives operating on a `gfx_surface_t`. The surface is a
plain public struct the caller owns by value — no singleton, no allocation.

## Scope

**In scope:** surfaces (on-screen + future off-screen), pixel formats, primitives
(pixel, filled rectangle, mono-bitmap blit, scanline-copy scroll), clipping.

**Out of scope (consumer's job):** fonts, text layout, console/terminal
semantics, input, mode-setting, device probing, widgets, UI.

## Headers

| Header | Purpose |
|--------|---------|
| `<gfx/surface.h>` | `gfx_surface_t` type + `gfx_surface_init` |
| `<gfx/draw.h>` | pixel drawing operations |

## Notes

`gfx_surface_scroll` reads back write-combining framebuffer memory (one linear
pass per scanline). This is slower than a private character buffer in the
consumer, but far cheaper than re-blitting thousands of glyphs. If profiling
shows this is a bottleneck, add a character cache in the `fb_console` layer.
