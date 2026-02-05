cmake_minimum_required(VERSION 3.20)

# Phase 1: Include JANUS platform detection BEFORE project() for cross-compiler setup
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(JanusPlatform)

# Enable appropriate assembler based on target architecture
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    project(JANUS VERSION 0.1.0 LANGUAGES C ASM_NASM)
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    project(JANUS VERSION 0.1.0 LANGUAGES C ASM)
else()
    project(JANUS VERSION 0.1.0 LANGUAGES C)
endif()

# Phase 2: Configure compiler flags after project() sets CMAKE_C_COMPILER_ID
include(JanusPlatform)

# Print build configuration
message(STATUS "========================================")
message(STATUS "JANUS Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target Architecture: ${JANUS_TARGET_ARCH}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER} (NASM)")
else()
    message(STATUS "Assembler: ${CMAKE_ASM_COMPILER} (GAS)")
endif()
message(STATUS "========================================")
message(STATUS "Available targets:")
message(STATUS "  ninja           - Build kernel.elf")
message(STATUS "  ninja iso       - Create Limine ISO (primary)")
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    message(STATUS "  ninja iso-grub  - Create GRUB ISO (educational, x86_64 only)")
endif()
message(STATUS "  ninja run       - Boot Limine ISO in QEMU")
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    message(STATUS "  ninja run-grub  - Boot GRUB ISO in QEMU (x86_64 only)")
endif()
message(STATUS "  ninja run-uefi  - Boot ISO in UEFI mode (requires OVMF)")
message(STATUS "  ninja debug     - Boot Limine ISO with GDB server on :1234")
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    message(STATUS "  ninja debug-grub - Boot GRUB ISO with GDB server (x86_64 only)")
endif()
message(STATUS "========================================")

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Allow GNU extension
set(CMAKE_C_EXTENSIONS ON)

# Generate build timestamp
string(TIMESTAMP JANUS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP JANUS_BUILD_TIME "%H:%M:%S")

# Copy compile_commands.json to project root for clangd
add_custom_target(
    copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to project root"
)

add_subdirectory(kernel)

# Setup all custom targets (iso, run, debug, etc.)
include(JanusTargets)
janus_setup_targets()
