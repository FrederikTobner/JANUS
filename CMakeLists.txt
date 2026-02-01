cmake_minimum_required(VERSION 3.20)
project(JANUS VERSION 0.1.0 LANGUAGES C ASM_NASM)

# Print build configuration
message(STATUS "========================================")
message(STATUS "JANUS Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER} (version ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER}")
message(STATUS "========================================")
message(STATUS "Available targets:")
message(STATUS "  ninja           - Build kernel.elf")
message(STATUS "  ninja iso       - Create Limine ISO (primary)")
message(STATUS "  ninja iso-grub  - Create GRUB ISO (educational, x86_64 only)")
message(STATUS "  ninja run       - Boot Limine ISO in QEMU")
message(STATUS "  ninja run-grub  - Boot GRUB ISO in QEMU (x86_64 only)")
message(STATUS "  ninja run-uefi  - Boot ISO in UEFI mode (requires OVMF)")
message(STATUS "  ninja debug     - Boot Limine ISO with GDB server on :1234")
message(STATUS "  ninja debug-grub - Boot GRUB ISO with GDB server (x86_64 only)")
message(STATUS "========================================")

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Allow GNU extension
set(CMAKE_C_EXTENSIONS ON)  

# Include JANUS platform detection and helper functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(JanusPlatform)

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

# TODO: Consider moving this and the display argument logic to JanusPlatform.cmake
set(JANUS_ISO_NAME "janus_${JANUS_TARGET_ARCH}.iso")

# Using i3 window manager we need to use the SDL display for QEMU to show the window
if(DEFINED ENV{XDG_CURRENT_DESKTOP} AND "$ENV{XDG_CURRENT_DESKTOP}" STREQUAL "i3")
    find_program(SDL2_CONFIG_EXECUTABLE sdl2-config)
    find_program(PKG_CONFIG_EXECUTABLE pkg-config)
    set(QEMU_DISPLAY_ARG)
    if(SDL2_CONFIG_EXECUTABLE)
       set(QEMU_DISPLAY_ARG -display sdl)
    elseif(PKG_CONFIG_EXECUTABLE)
        execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --exists gtk+-3.0
            RESULT_VARIABLE GTK3_NOT_FOUND
        )
        if(GTK3_NOT_FOUNDND)
            message(WARNING "Neither SDL2 nor GTK3 development libraries found. QEMU display argument will not be set. QEMU may start a VNC server under i3.")
        else()
            set(QEMU_DISPLAY_ARG -display gtk)
        endif()
    else()
        message(WARNING "Neither SDL2 nor GTK3 development libraries found. QEMU display argument will not be set. QEMU may start a VNC server under i3.")
    endif()
else()
    set(QEMU_DISPLAY_ARG)
endif()

# ISO creation target
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    # Configure GRUB config if multiboot2 is enabled
    if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
            ${CMAKE_BINARY_DIR}/grub.cfg
            @ONLY
        )
    endif()

    # Configure Limine config if limine is enabled (limine.conf for v8+)
    if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/limine/limine.conf.in
            ${CMAKE_BINARY_DIR}/limine.conf
            @ONLY
        )
        
        # Check for Limine installation and detect path
        find_program(LIMINE_EXECUTABLE limine)
        set(LIMINE_DATA_PATHS
            /usr/share/limine
            /usr/local/share/limine
            /opt/limine/share/limine
        )
        set(LIMINE_DATA_DIR "LIMINE_DATA_DIR-NOTFOUND")
        foreach(path ${LIMINE_DATA_PATHS})
            if(EXISTS "${path}/limine-bios.sys")
                set(LIMINE_DATA_DIR ${path})
                break()
            endif()
        endforeach()
        
        if(LIMINE_DATA_DIR STREQUAL "LIMINE_DATA_DIR-NOTFOUND")
            message(WARNING "Limine data files not found. The 'iso' target will fail.\n"
                "Install Limine: https://github.com/limine-bootloader/limine\n"
                "  Arch: yay -S limine\n"
                "  Manual: git clone --depth 1 https://github.com/limine-bootloader/limine.git && cd limine && make install\n"
                "Use 'ninja iso-grub' for GRUB-based ISO instead.")
        else()
            message(STATUS "Limine data directory: ${LIMINE_DATA_DIR}")
        endif()
    endif()

    # ISO image creation
    # Primary ISO uses Limine (supports both boot protocols, shows menu)
    # GRUB ISO is available as a separate target for educational purposes
    set(JANUS_ISO_GRUB_NAME "janus-grub.iso")

    # Primary ISO: Limine bootloader with Limine protocol (higher-half kernel)
    if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
        if(LIMINE_DATA_DIR AND NOT LIMINE_DATA_DIR STREQUAL "LIMINE_DATA_DIR-NOTFOUND")
            add_custom_target(iso
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/limine
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/EFI/BOOT
                # Use kernel-limine.elf (higher-half)
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-limine.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
                # Limine v8+ uses limine.conf at /boot/limine.conf
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/limine.conf ${CMAKE_BINARY_DIR}/iso/boot/limine.conf
                # Copy Limine binaries from detected installation
                COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios.sys ${CMAKE_BINARY_DIR}/iso/boot/limine/
                COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios-cd.bin ${CMAKE_BINARY_DIR}/iso/boot/limine/
                COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-uefi-cd.bin ${CMAKE_BINARY_DIR}/iso/boot/limine/
                COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/BOOTX64.EFI ${CMAKE_BINARY_DIR}/iso/EFI/BOOT/
                # Create ISO with xorriso
                COMMAND xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin
                    -no-emul-boot -boot-load-size 4 -boot-info-table
                    --efi-boot boot/limine/limine-uefi-cd.bin
                    -efi-boot-part --efi-boot-image --protective-msdos-label
                    ${CMAKE_BINARY_DIR}/iso -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                # Install Limine to ISO
                COMMAND limine bios-install ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                DEPENDS kernel-limine.elf ${CMAKE_BINARY_DIR}/limine.conf
                COMMENT "Creates a bootable ISO image (Limine protocol)"
            )
        else()
            # Limine not found - provide helpful error
            add_custom_target(iso
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: Limine bootloader not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install Limine: https://github.com/limine-bootloader/limine"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: yay -S limine"
                COMMAND ${CMAKE_COMMAND} -E echo "  Manual: clone repo, then make install"
                COMMAND ${CMAKE_COMMAND} -E echo "Or use: ninja iso-grub"
                COMMAND ${CMAKE_COMMAND} -E false
                COMMENT "Limine bootloader not installed"
            )
        endif()
    else()
        message(FATAL_ERROR "Limine boot protocol required for primary ISO. Add 'limine' to JANUS_BOOT_PROTOCOLS.")
    endif()

    # Secondary ISO: GRUB bootloader with Multiboot2 protocol (x86_64 only)
    if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
        add_custom_target(iso-grub
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso-grub
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso-grub/boot/grub
            # Use kernel-multiboot2.elf (physical addresses)
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-multiboot2.elf> ${CMAKE_BINARY_DIR}/iso-grub/boot/kernel.elf
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso-grub/boot/grub/grub.cfg
            COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME} ${CMAKE_BINARY_DIR}/iso-grub/
            DEPENDS kernel-multiboot2.elf ${CMAKE_BINARY_DIR}/grub.cfg
            COMMENT "Creates a bootable ISO image (GRUB/Multiboot2)"
        )
    else()
        add_custom_target(iso-grub
            COMMAND ${CMAKE_COMMAND} -E echo "GRUB ISO requires 'multiboot2' in JANUS_BOOT_PROTOCOLS"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "GRUB ISO not available - multiboot2 protocol not enabled"
        )
    endif()
else()
    add_custom_target(iso
        COMMAND ${CMAKE_COMMAND} -E echo "ISO packaging not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "ISO packaging not implemented for this architecture"
    )
    add_custom_target(iso-grub
        COMMAND ${CMAKE_COMMAND} -E echo "GRUB ISO only available for x86_64"
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "GRUB ISO only available for x86_64"
    )
endif()

# Custom target to run JANUS in QEMU (boots Limine ISO, select protocol from menu)
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    add_custom_target(run
        COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME} 
            -boot d -serial stdio -m 256M ${QEMU_DISPLAY_ARG}
        DEPENDS iso
        USES_TERMINAL
        COMMENT "Boots JANUS Limine ISO (select boot protocol from menu)"
    )

    # Run GRUB ISO (Multiboot2 only - educational)
    add_custom_target(run-grub
        COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME}
            -boot d -serial stdio -m 256M ${QEMU_DISPLAY_ARG}
        DEPENDS iso-grub
        USES_TERMINAL
        COMMENT "Boots JANUS GRUB ISO (Multiboot2 - educational)"
    )

    # Direct kernel load - NOTE: QEMU's -kernel only supports Multiboot1, not Multiboot2
    # This target is provided for future use if Multiboot1 is added, or as documentation.
    # For now, use 'ninja run' which boots via ISO (works with both protocols).
    add_custom_target(run-elf
        COMMAND ${CMAKE_COMMAND} -E echo "Note: QEMU -kernel only supports Multiboot1, not Multiboot2."
        COMMAND ${CMAKE_COMMAND} -E echo "Use 'ninja run' to boot via ISO instead."
        COMMAND ${CMAKE_COMMAND} -E false
        DEPENDS kernel.elf
        USES_TERMINAL
        COMMENT "Direct kernel boot not supported (QEMU requires Multiboot1)"
    )

    # UEFI boot (requires OVMF)
    add_custom_target(run-uefi
        COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            -boot d -serial stdio -m 256M ${QEMU_DISPLAY_ARG}
            -bios /usr/share/OVMF/OVMF_CODE.fd
        DEPENDS iso
        USES_TERMINAL
        COMMENT "Boots JANUS ISO in UEFI mode (requires OVMF)"
    )
else()
    add_custom_target(run
        COMMAND ${CMAKE_COMMAND} -E echo "Run target not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "Run target not implemented for this architecture"
    )
    add_custom_target(run-grub
        COMMAND ${CMAKE_COMMAND} -E echo "GRUB run target only available for x86_64"
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "GRUB run target only available for x86_64"
    )
    add_custom_target(run-elf
        COMMAND ${CMAKE_COMMAND} -E echo "Run-elf target not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "Run-elf target not implemented for this architecture"
    )
    add_custom_target(run-uefi
        COMMAND ${CMAKE_COMMAND} -E echo "Run-uefi target not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "Run-uefi target not implemented for this architecture"
    )
endif()

# Debug targets (with GDB server on :1234)
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    add_custom_target(debug
        COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            -boot d -serial stdio -m 256M -s -S ${QEMU_DISPLAY_ARG}
        DEPENDS iso
        USES_TERMINAL
        COMMENT "Boots JANUS Limine ISO with GDB server on :1234"
    )

    # Debug GRUB ISO (Multiboot2 only - educational)
    add_custom_target(debug-grub
        COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME}
            -boot d -serial stdio -m 256M -s -S ${QEMU_DISPLAY_ARG}
        DEPENDS iso-grub
        USES_TERMINAL
        COMMENT "Boots JANUS GRUB ISO with GDB server on :1234 (Multiboot2 - educational)"
    )

    add_custom_target(debug-elf
        COMMAND ${CMAKE_COMMAND} -E echo "Note: QEMU -kernel only supports Multiboot1, not Multiboot2."
        COMMAND ${CMAKE_COMMAND} -E echo "Use 'ninja debug' to boot via ISO instead."
        COMMAND ${CMAKE_COMMAND} -E false
        DEPENDS kernel.elf
        USES_TERMINAL
        COMMENT "Direct kernel debug not supported (QEMU requires Multiboot1)"
    )
else()
    add_custom_target(debug
        COMMAND ${CMAKE_COMMAND} -E echo "Debug target not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "Debug target not implemented for this architecture"
    )
    add_custom_target(debug-grub
        COMMAND ${CMAKE_COMMAND} -E echo "GRUB debug target only available for x86_64"
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "GRUB debug target only available for x86_64"
    )
    add_custom_target(debug-elf
        COMMAND ${CMAKE_COMMAND} -E echo "Debug-elf target not implemented for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}'."
        COMMAND ${CMAKE_COMMAND} -E false
        USES_TERMINAL
        COMMENT "Debug-elf target not implemented for this architecture"
    )
endif()

