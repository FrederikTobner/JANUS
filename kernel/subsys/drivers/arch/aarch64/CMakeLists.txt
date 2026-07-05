# aarch64 architecture implementation for drivers subsystem

janus_add_arch_subsys(drivers
    SOURCES
        serial.c
        ../shared/framebuffer.c
        ../shared/fb_console.c
        console.c
    DEPENDENCIES
        janus_asm
        gfx
        janus_contract_display
        page_tables
        kio
)
