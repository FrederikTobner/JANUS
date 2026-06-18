# Limine boot protocol implementation
# Plain library (not janus_add_subsys) — sub-library of the boot subsystem

add_library(boot_limine STATIC
    limine_boot.c
)

janus_register(boot_limine PROTOCOL_LIB "boot")

target_link_libraries(boot_limine PUBLIC boot)
target_include_directories(boot_limine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

janus_apply_compile_flags(boot_limine)