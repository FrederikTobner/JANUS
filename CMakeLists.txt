cmake_minimum_required(VERSION 3.20)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Allow GNU extension
set(CMAKE_C_EXTENSIONS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(JANUS VERSION 0.1.0 LANGUAGES C)

# JANUS_TARGET_ARCH: set by toolchain file, preset, or -D flag.
# Must be checked AFTER project() because toolchain files aren't loaded until then.
if(NOT DEFINED JANUS_TARGET_ARCH)
    set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86_64 or aarch64)")
    message(STATUS "JANUS_TARGET_ARCH not set, defaulting to x86_64")
    message(STATUS "For cross-compilation, use: cmake --preset <preset-name>")
endif()

if(NOT JANUS_TARGET_ARCH STREQUAL "x86_64" AND NOT JANUS_TARGET_ARCH STREQUAL "aarch64")
    message(FATAL_ERROR "Invalid JANUS_TARGET_ARCH: ${JANUS_TARGET_ARCH}. Must be 'x86_64' or 'aarch64'.")
endif()

if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    enable_language(ASM_NASM)
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    enable_language(ASM)
endif()

include(platform/Detection)
include(platform/CompilerFlags)

add_custom_target(
    copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to project root"
)

add_subdirectory(kernel)

message(STATUS "================================================================================")
message(STATUS "JANUS Build Configuration")
message(STATUS "================================================================================")
message(STATUS "Build Type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "Target Arch:     ${JANUS_TARGET_ARCH}")
message(STATUS "Compiler:        ${CMAKE_C_COMPILER_ID} (${CMAKE_C_COMPILER})")
message(STATUS "Linker:          ${CMAKE_LINKER}")
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    message(STATUS "Assembler:       ${CMAKE_ASM_NASM_COMPILER} (NASM)")
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    message(STATUS "Assembler:       ${CMAKE_ASM_COMPILER} (GAS)")
endif()
message(STATUS "Boot Protocols:  ${JANUS_BOOT_PROTOCOLS}")
message(STATUS "================================================================================")

include(Targets)
janus_setup_targets()
janus_print_targets()
