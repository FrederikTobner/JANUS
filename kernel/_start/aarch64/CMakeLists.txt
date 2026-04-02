# Entry point: aarch64
# Creates kernel binary for Limine boot protocol on ARM64
# - kernel-limine.elf: Limine protocol (higher-half, UEFI boot)
#
# Note: Multiboot2 is x86_64-specific and not supported on aarch64.
# GAS (GNU Assembler) is used instead of NASM for aarch64 assembly.

# Limine requests in portable C (shared across architectures)
add_library(_start_limine_requests OBJECT
    ${CMAKE_SOURCE_DIR}/kernel/_start/common/limine_requests.c
)
target_include_directories(_start_limine_requests PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/protocol/limine
)
target_compile_options(_start_limine_requests PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})

# aarch64 specific Limine entry point (GAS assembly)
add_library(_start_limine OBJECT
    limine/entry.S
)
target_compile_options(_start_limine PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})

# Build kernel-limine.elf with Limine linker script
include(JanusKernel)
janus_add_kernel(
    TARGET kernel-limine.elf
    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/limine/linker.ld
    DEPENDENCIES drivers boot fmt boot_limine
    OBJECTS _start_limine _start_limine_requests kmain
)
