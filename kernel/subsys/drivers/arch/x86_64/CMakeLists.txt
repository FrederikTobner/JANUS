# x86_64 architecture implementation for drivers subsystem

janus_add_arch_subsys(drivers
    SOURCES
        serial.c
        tty.c
        ../shared/framebuffer.c
)
