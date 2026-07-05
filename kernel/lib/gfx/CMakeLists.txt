# CMake configuration for the gfx library
#
# Provides:
#   gfx_surface_t                             — drawable pixel surface (public struct)
#   gfx_surface_init                          — surface initializer
#   gfx_surface_put_pixel / fill_rect         — pixel primitives
#   gfx_surface_blit_mono                     — 1-bpp mono-bitmap blit
#   gfx_surface_scroll                        — scanline-copy scroll + fill

janus_add_library(gfx
    SOURCES
        src/draw.c
)
