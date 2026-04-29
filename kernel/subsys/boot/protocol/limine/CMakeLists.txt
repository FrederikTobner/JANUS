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

target_compile_options(boot_limine PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(boot_limine PRIVATE ${JANUS_COMPILE_OPTIONS_DEBUG})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(boot_limine PRIVATE ${JANUS_COMPILE_OPTIONS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(boot_limine PRIVATE ${JANUS_COMPILE_OPTIONS_MINSIZEREL})
endif()
