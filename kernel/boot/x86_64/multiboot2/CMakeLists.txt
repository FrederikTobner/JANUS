# Boot module: x86_64 + multiboot2

enable_language(ASM_NASM)

# NASM configuration
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Build boot assembly files as object library
add_library(boot OBJECT
    multiboot2.asm
    boot.asm
    verify.c
)

target_include_directories(boot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_options(boot PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${TINYOS_COMMON_FLAGS}>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(boot PRIVATE $<$<COMPILE_LANGUAGE:C>:${TINYOS_DEBUG_FLAGS}>)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(boot PRIVATE $<$<COMPILE_LANGUAGE:C>:${TINYOS_RELEASE_FLAGS}>)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(boot PRIVATE $<$<COMPILE_LANGUAGE:C>:${TINYOS_MINSIZEREL_FLAGS}>)
endif()

message(STATUS "  Added module: boot (${TINYOS_TARGET_ARCH}/${TINYOS_BOOT_PROTOCOL})")