# Entry point: x86_64
# Creates separate kernel binaries for each boot protocol:
# - kernel-limine.elf: Limine protocol (higher-half, portable to ARM)
# - kernel-multiboot2.elf: Multiboot2/GRUB protocol (x86_64 only)

enable_language(ASM_NASM)

# NASM configuration
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Common x86_64 startup code (shared by all protocols)
add_library(_start_common OBJECT
    common/gdt64.asm
)

if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
    # Limine requests in portable C (shared across architectures)
    add_library(_start_limine_requests OBJECT
        ${CMAKE_SOURCE_DIR}/kernel/_start/common/limine_requests.c
    )
    target_include_directories(_start_limine_requests PRIVATE
        ${CMAKE_SOURCE_DIR}/kernel/include
        ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/protocol/limine
    )
    target_compile_options(_start_limine_requests PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})

    # x86_64 specific Limine entry point (assembly)
    add_library(_start_limine OBJECT
        limine/entry.asm
    )
    
    # Build kernel-limine.elf with Limine linker script
    janus_add_kernel(
        TARGET kernel-limine.elf
        LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/limine/linker.ld
        DEPENDENCIES drivers boot fmt boot_limine kio mm
        OBJECTS _start_common _start_limine _start_limine_requests kmain
    )
endif()

if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
    add_library(_start_multiboot2 OBJECT
        multiboot2/entry.asm
        multiboot2/header.asm
        multiboot2/paging.asm
    )
    
    # Build kernel-multiboot2.elf with Multiboot2 linker script
    janus_add_kernel(
        TARGET kernel-multiboot2.elf
        LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/multiboot2/linker.ld
        DEPENDENCIES drivers boot fmt boot_multiboot2 kio mm
        OBJECTS _start_common _start_multiboot2 kmain
    )
endif()
