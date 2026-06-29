# aarch64 architecture implementation for interrupts subsystem

janus_add_arch_subsys(interrupts
    SOURCES
        exceptions.c
    DEPENDENCIES
        kio
)
