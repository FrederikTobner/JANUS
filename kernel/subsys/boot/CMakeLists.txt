# Boot module: x86_64 + multiboot2
# Protocol verification and boot info handling

file(GLOB_RECURSE BOOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# Build boot as static library
janus_add_subsys(boot 
SOURCES
   info.c
   verify.c 
)

target_include_directories(boot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/internal
)
