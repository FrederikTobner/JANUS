cmake_minimum_required(VERSION 3.20)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Allow GNU extension
set(CMAKE_C_EXTENSIONS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(JANUS VERSION 0.1.0 LANGUAGES C)

enable_testing()

if(NOT DEFINED JANUS_TARGET_ARCH)
    set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86_64 or aarch64)")
    message(STATUS "JANUS_TARGET_ARCH not set, defaulting to x86_64
    For cross-compilation, use: cmake --preset <preset-name>")
endif()

if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    enable_language(ASM_NASM)
    set(JANUS_ASSEMBLER ${CMAKE_ASM_NASM_COMPILER})
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    enable_language(ASM)
    set(JANUS_ASSEMBLER ${CMAKE_ASM_COMPILER})
else()
    message(FATAL_ERROR "Invalid JANUS_TARGET_ARCH: ${JANUS_TARGET_ARCH}. Must be 'x86_64' or 'aarch64'.")
endif()

include(platform/Detection)
include(arch/${JANUS_TARGET_ARCH}/BootProtocols)

add_subdirectory(kernel)

message(STATUS "Printing JANUS Build Configuration... 
        Build Type: ${CMAKE_BUILD_TYPE}
        Target Arch: ${JANUS_TARGET_ARCH}
        Compiler: ${CMAKE_C_COMPILER_ID} (${CMAKE_C_COMPILER})
        Linker: ${CMAKE_LINKER}
        Assembler: ${JANUS_ASSEMBLER}
        Boot Protocols: ${JANUS_BOOT_PROTOCOLS}
")

include(image/Targets)
janus_setup_targets()
janus_print_targets()
