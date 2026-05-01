# CMake configuration for the display library
#
# Provides:
#   display_info_t / display_mode_t — shared display config type (header-only)
#   display_fb_t                    — framebuffer view handle
#   display_put_pixel / display_fill_rect / display_blit_glyph — pixel primitives

janus_add_library(display
    SOURCES
        src/fb.c
)
