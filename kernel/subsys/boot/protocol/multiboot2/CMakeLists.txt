# Multiboot2 boot protocol implementation (x86_64 only)
# Plain library (not janus_add_subsys) — sub-library of the boot subsystem

add_library(boot_multiboot2 STATIC
    multiboot2_boot.c
)

target_link_libraries(boot_multiboot2 PUBLIC boot)
target_include_directories(boot_multiboot2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(boot_multiboot2 PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(boot_multiboot2 PRIVATE ${JANUS_COMPILE_OPTIONS_DEBUG})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(boot_multiboot2 PRIVATE ${JANUS_COMPILE_OPTIONS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(boot_multiboot2 PRIVATE ${JANUS_COMPILE_OPTIONS_MINSIZEREL})
endif()
