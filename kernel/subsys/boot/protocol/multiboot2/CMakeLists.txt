# Multiboot2 boot protocol implementation (x86_64 only)
# Plain library (not janus_add_subsys) — sub-library of the boot subsystem

add_library(boot_multiboot2 STATIC
    multiboot2_boot.c
)

janus_register(boot_multiboot2 PROTOCOL_LIB "boot")

target_link_libraries(boot_multiboot2 PUBLIC boot)
target_include_directories(boot_multiboot2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

janus_apply_compile_flags(boot_multiboot2)

