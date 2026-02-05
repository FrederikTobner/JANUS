# Portable driver logic

janus_add_subsys(drivers SOURCES
    tty.c
)

# Note: arch/ is processed by janus_add_subsys via add_subdirectory(arch)
