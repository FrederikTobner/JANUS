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
    add_library(_start_limine OBJECT
        limine/entry.asm
        limine/requests.asm
    )
    
    # Build kernel-limine.elf with Limine linker script
    janus_link_kernel(
        TARGET kernel-limine.elf
        LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/link-limine.ld
        LIBRARIES drivers boot
        OBJECTS _start_common _start_limine kmain
    )
endif()

if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
    add_library(_start_multiboot2 OBJECT
        multiboot2/entry.asm
        multiboot2/header.asm
    )
    
    # Build kernel-multiboot2.elf with Multiboot2 linker script
    janus_link_kernel(
        TARGET kernel-multiboot2.elf
        LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/link-multiboot2.ld
        LIBRARIES drivers boot
        OBJECTS _start_common _start_multiboot2 kmain
    )
endif()
