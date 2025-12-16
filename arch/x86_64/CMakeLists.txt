# x86_64 architecture support

# Automatically find all source files
file(GLOB_RECURSE ARCH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE ARCH_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE ARCH_ASM "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

tinyos_add_module(arch_x86_64
    SOURCES
        ${ARCH_SOURCES}
        ${ARCH_ASM}
)
