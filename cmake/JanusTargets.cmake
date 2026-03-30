#[[
    JanusTargets.cmake - Custom build targets for JANUS

    This module defines all custom targets for:
    - ISO image creation (Limine and GRUB)
    - Running in QEMU
    - Debugging with GDB

    It also handles dependency checking for required external tools.
]]

# Check for required tools and provide helpful error messages
function(janus_check_required_tools)
    set(MISSING_TOOLS "")
    set(INSTALL_HINTS "")

    # NASM (assembler)
    find_program(NASM_EXECUTABLE nasm)
    if(NOT NASM_EXECUTABLE)
        list(APPEND MISSING_TOOLS "nasm")
        list(APPEND INSTALL_HINTS "  nasm:\n    Arch: sudo pacman -S nasm\n    Debian/Ubuntu: sudo apt install nasm")
    endif()

    # xorriso (ISO creation)
    find_program(XORRISO_EXECUTABLE xorriso)
    if(NOT XORRISO_EXECUTABLE)
        list(APPEND MISSING_TOOLS "xorriso")
        list(APPEND INSTALL_HINTS "  xorriso:\n    Arch: sudo pacman -S libisoburn\n    Debian/Ubuntu: sudo apt install xorriso")
    endif()

    # QEMU (emulation)
    if(JANUS_TARGET_ARCH STREQUAL "x86_64")
        find_program(QEMU_EXECUTABLE qemu-system-x86_64)
        if(NOT QEMU_EXECUTABLE)
            list(APPEND MISSING_TOOLS "qemu-system-x86_64")
            list(APPEND INSTALL_HINTS "  qemu-system-x86_64:\n    Arch: sudo pacman -S qemu-system-x86\n    Debian/Ubuntu: sudo apt install qemu-system-x86")
        endif()
    elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
        find_program(QEMU_EXECUTABLE qemu-system-aarch64)
        if(NOT QEMU_EXECUTABLE)
            list(APPEND MISSING_TOOLS "qemu-system-aarch64")
            list(APPEND INSTALL_HINTS "  qemu-system-aarch64:\n    Arch: sudo pacman -S qemu-system-aarch64\n    Debian/Ubuntu: sudo apt install qemu-system-arm")
        endif()
    endif()

    # grub-mkrescue (GRUB ISO creation)
    find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)
    if(NOT GRUB_MKRESCUE_EXECUTABLE)
        list(APPEND MISSING_TOOLS "grub-mkrescue")
        list(APPEND INSTALL_HINTS "  grub-mkrescue:\n    Arch: sudo pacman -S grub\n    Debian/Ubuntu: sudo apt install grub-pc-bin grub-common xorriso mtools")
    endif()

    # Limine (Limine bootloader)
    find_program(LIMINE_EXECUTABLE limine)
    if(NOT LIMINE_EXECUTABLE)
        list(APPEND MISSING_TOOLS "limine")
        list(APPEND INSTALL_HINTS "  limine:\n    Arch: sudo pacman -S limine\n    Debian/Ubuntu: Build from source:\n      git clone --depth 1 https://github.com/limine-bootloader/limine.git\n      cd limine && make && sudo make install")
    endif()

    # Report missing tools
    if(MISSING_TOOLS)
        string(REPLACE ";" ", " MISSING_LIST "${MISSING_TOOLS}")
        string(REPLACE ";" "\n" HINTS_TEXT "${INSTALL_HINTS}")
        message(WARNING 
            "The following tools are not installed: ${MISSING_LIST}\n"
            "Some build targets may fail.\n\n"
            "Installation instructions:\n${HINTS_TEXT}\n"
        )
    endif()

    # Export found tools to parent scope
    set(JANUS_XORRISO_FOUND ${XORRISO_EXECUTABLE} PARENT_SCOPE)
    set(JANUS_QEMU_FOUND ${QEMU_EXECUTABLE} PARENT_SCOPE)
    set(JANUS_GRUB_MKRESCUE_FOUND ${GRUB_MKRESCUE_EXECUTABLE} PARENT_SCOPE)
    set(JANUS_LIMINE_FOUND ${LIMINE_EXECUTABLE} PARENT_SCOPE)
endfunction()

function(janus_find_limine)
    # Check for Limine executable
    find_program(LIMINE_EXECUTABLE limine)
    
    # Search for Limine data files
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
        message(WARNING 
            "Limine data files not found. The 'iso' target will fail.\n"
            "Install Limine bootloader:\n"
            "  Arch:          sudo pacman -S limine\n"
            "  Debian/Ubuntu: Build from source:\n"
            "                 git clone --depth 1 https://github.com/limine-bootloader/limine.git\n"
            "                 cd limine && make && sudo make install\n"
            "\n"
            "Alternative: Use 'ninja iso-grub' for GRUB-based ISO instead."
        )
    endif()

    # Export to parent scope
    set(LIMINE_DATA_DIR ${LIMINE_DATA_DIR} PARENT_SCOPE)
    set(LIMINE_EXECUTABLE ${LIMINE_EXECUTABLE} PARENT_SCOPE)
endfunction()

function(janus_configure_qemu_display)
    # Using i3 window manager we need to use the SDL display for QEMU to show the window
    if(DEFINED ENV{XDG_CURRENT_DESKTOP} AND "$ENV{XDG_CURRENT_DESKTOP}" STREQUAL "i3")
        find_program(SDL2_CONFIG_EXECUTABLE sdl2-config)
        find_program(PKG_CONFIG_EXECUTABLE pkg-config)
        
        if(SDL2_CONFIG_EXECUTABLE)
            set(QEMU_DISPLAY_ARG -display sdl PARENT_SCOPE)
        elseif(PKG_CONFIG_EXECUTABLE)
            execute_process(
                COMMAND ${PKG_CONFIG_EXECUTABLE} --exists gtk+-3.0
                RESULT_VARIABLE GTK3_NOT_FOUND
            )
            if(NOT GTK3_NOT_FOUND)
                set(QEMU_DISPLAY_ARG -display gtk PARENT_SCOPE)
            else()
                message(WARNING "Neither SDL2 nor GTK3 development libraries found. "
                    "QEMU display argument will not be set. QEMU may start a VNC server under i3.")
                set(QEMU_DISPLAY_ARG "" PARENT_SCOPE)
            endif()
        else()
            message(WARNING "Neither SDL2 nor GTK3 development libraries found. "
                "QEMU display argument will not be set. QEMU may start a VNC server under i3.")
            set(QEMU_DISPLAY_ARG "" PARENT_SCOPE)
        endif()
    else()
        set(QEMU_DISPLAY_ARG "" PARENT_SCOPE)
    endif()
endfunction()

function(janus_add_iso_targets)
    set(JANUS_ISO_NAME "janus_${JANUS_TARGET_ARCH}.iso")
    set(JANUS_ISO_GRUB_NAME "janus_${JANUS_TARGET_ARCH}_grub.iso")

    # Configure Limine config (works for both architectures)
    if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/limine/limine.conf.in
            ${CMAKE_BINARY_DIR}/limine.conf
            @ONLY
        )
    endif()

    if(JANUS_TARGET_ARCH STREQUAL "aarch64")
        # aarch64: UEFI-only ISO (no BIOS on ARM)
        add_custom_target(iso-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: GRUB ISO only available for x86_64"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "GRUB ISO only available for x86_64"
        )

        if(LIMINE_DATA_DIR AND NOT LIMINE_DATA_DIR STREQUAL "LIMINE_DATA_DIR-NOTFOUND")
            if(NOT JANUS_XORRISO_FOUND)
                add_custom_target(iso
                    COMMAND ${CMAKE_COMMAND} -E echo "ERROR: xorriso not installed."
                    COMMAND ${CMAKE_COMMAND} -E echo "Install xorriso:"
                    COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S libisoburn"
                    COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install xorriso"
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "xorriso not installed"
                )
            else()
                # aarch64 UEFI-only ISO with Limine
                add_custom_target(iso
                    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/limine
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/EFI/BOOT
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-limine.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
                    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/limine.conf ${CMAKE_BINARY_DIR}/iso/boot/limine.conf
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-uefi-cd.bin ${CMAKE_BINARY_DIR}/iso/boot/limine/
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/BOOTAA64.EFI ${CMAKE_BINARY_DIR}/iso/EFI/BOOT/
                    COMMAND xorriso -as mkisofs
                        --efi-boot boot/limine/limine-uefi-cd.bin
                        -efi-boot-part --efi-boot-image --protective-msdos-label
                        ${CMAKE_BINARY_DIR}/iso -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    DEPENDS kernel-limine.elf ${CMAKE_BINARY_DIR}/limine.conf
                    COMMENT "Creates a bootable UEFI ISO image for aarch64 (Limine)"
                )
            endif()
        else()
            add_custom_target(iso
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: Limine bootloader not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install Limine:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S limine"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: Build from source"
                COMMAND ${CMAKE_COMMAND} -E false
                COMMENT "Limine bootloader not installed"
            )
        endif()
        return()
    endif()

    # x86_64: Full BIOS + UEFI ISO support
    # Configure GRUB config if multiboot2 is enabled
    if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
            ${CMAKE_BINARY_DIR}/grub.cfg
            @ONLY
        )
    endif()

    # Primary ISO: Limine bootloader
    if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
        if(LIMINE_DATA_DIR AND NOT LIMINE_DATA_DIR STREQUAL "LIMINE_DATA_DIR-NOTFOUND")
            if(NOT JANUS_XORRISO_FOUND)
                add_custom_target(iso
                    COMMAND ${CMAKE_COMMAND} -E echo "ERROR: xorriso not installed."
                    COMMAND ${CMAKE_COMMAND} -E echo "Install xorriso:"
                    COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S libisoburn"
                    COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install xorriso"
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "xorriso not installed"
                )
            else()
                add_custom_target(iso
                    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/limine
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/EFI/BOOT
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-limine.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
                    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/limine.conf ${CMAKE_BINARY_DIR}/iso/boot/limine.conf
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios.sys ${CMAKE_BINARY_DIR}/iso/boot/limine/
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios-cd.bin ${CMAKE_BINARY_DIR}/iso/boot/limine/
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-uefi-cd.bin ${CMAKE_BINARY_DIR}/iso/boot/limine/
                    COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/BOOTX64.EFI ${CMAKE_BINARY_DIR}/iso/EFI/BOOT/
                    COMMAND xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin
                        -no-emul-boot -boot-load-size 4 -boot-info-table
                        --efi-boot boot/limine/limine-uefi-cd.bin
                        -efi-boot-part --efi-boot-image --protective-msdos-label
                        ${CMAKE_BINARY_DIR}/iso -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    COMMAND limine bios-install ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    DEPENDS kernel-limine.elf ${CMAKE_BINARY_DIR}/limine.conf
                    COMMENT "Creates a bootable ISO image (Limine protocol)"
                )
            endif()
        else()
            # Limine not found - provide helpful error
            add_custom_target(iso
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: Limine bootloader not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install Limine:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S limine"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: Build from source:"
                COMMAND ${CMAKE_COMMAND} -E echo "    git clone --depth 1 https://github.com/limine-bootloader/limine.git"
                COMMAND ${CMAKE_COMMAND} -E echo "    cd limine && make && sudo make install"
                COMMAND ${CMAKE_COMMAND} -E echo ""
                COMMAND ${CMAKE_COMMAND} -E echo "Alternative: Use 'ninja iso-grub' for GRUB-based ISO"
                COMMAND ${CMAKE_COMMAND} -E false
                COMMENT "Limine bootloader not installed"
            )
        endif()
    else()
        message(FATAL_ERROR "Limine boot protocol required for primary ISO. Add 'limine' to JANUS_BOOT_PROTOCOLS.")
    endif()

    # Secondary ISO: GRUB bootloader with Multiboot2
    if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
        if(NOT JANUS_GRUB_MKRESCUE_FOUND)
            add_custom_target(iso-grub
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: grub-mkrescue not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install GRUB tools:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S grub"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install grub-pc-bin grub-common xorriso mtools"
                COMMAND ${CMAKE_COMMAND} -E false
                COMMENT "grub-mkrescue not installed"
            )
        else()
            add_custom_target(iso-grub
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso-grub
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso-grub/boot/grub
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-multiboot2.elf> ${CMAKE_BINARY_DIR}/iso-grub/boot/kernel.elf
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso-grub/boot/grub/grub.cfg
                COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME} ${CMAKE_BINARY_DIR}/iso-grub/
                DEPENDS kernel-multiboot2.elf ${CMAKE_BINARY_DIR}/grub.cfg
                COMMENT "Creates a bootable ISO image (GRUB/Multiboot2)"
            )
        endif()
    else()
        add_custom_target(iso-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: GRUB ISO requires 'multiboot2' in JANUS_BOOT_PROTOCOLS"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "GRUB ISO not available - multiboot2 protocol not enabled"
        )
    endif()
endfunction()

function(janus_add_run_targets)
    set(JANUS_ISO_NAME "janus_${JANUS_TARGET_ARCH}.iso")
    set(JANUS_ISO_GRUB_NAME "janus_${JANUS_TARGET_ARCH}_grub.iso")

    # Check for appropriate QEMU binary
    if(NOT JANUS_QEMU_FOUND)
        if(JANUS_TARGET_ARCH STREQUAL "x86_64")
            add_custom_target(run
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: qemu-system-x86_64 not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install QEMU:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S qemu-system-x86"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install qemu-system-x86"
                COMMAND ${CMAKE_COMMAND} -E false
                USES_TERMINAL
                COMMENT "QEMU not installed"
            )
        else()
            add_custom_target(run
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: qemu-system-aarch64 not installed."
                COMMAND ${CMAKE_COMMAND} -E echo "Install QEMU:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S qemu-system-aarch64"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install qemu-system-arm"
                COMMAND ${CMAKE_COMMAND} -E false
                USES_TERMINAL
                COMMENT "QEMU not installed"
            )
        endif()
        add_custom_target(run-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        add_custom_target(run-elf
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        add_custom_target(run-uefi
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        return()
    endif()

    if(JANUS_TARGET_ARCH STREQUAL "aarch64")
        # aarch64: Run with qemu-system-aarch64 using UEFI and Limine ISO
        # Find UEFI firmware for aarch64
        set(AARCH64_UEFI_FIRMWARE "")
        if(EXISTS "/usr/share/AAVMF/AAVMF_CODE.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/AAVMF/AAVMF_CODE.fd")
        elseif(EXISTS "/usr/share/edk2/aarch64/QEMU_EFI.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/edk2/aarch64/QEMU_EFI.fd")
        elseif(EXISTS "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd")
        endif()

        if(AARCH64_UEFI_FIRMWARE)
            # QEMU debug logging options - logs to build directory
            # cpu_reset: log CPU resets; guest_errors: log guest errors
            # unimp: log unimplemented features; exec: log executed blocks
            set(QEMU_DEBUG_LOG "${CMAKE_BINARY_DIR}/qemu-debug.log")
            set(QEMU_DEBUG_OPTS -d cpu_reset,guest_errors,unimp -D ${QEMU_DEBUG_LOG})

            add_custom_target(run
                COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
                COMMAND qemu-system-aarch64 
                    -M virt
                    -cpu cortex-a72
                    -m 256M
                    -device ramfb
                    -serial stdio
                    -bios ${AARCH64_UEFI_FIRMWARE}
                    -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    -boot d
                    ${QEMU_DEBUG_OPTS}
                    ${QEMU_DISPLAY_ARG}
                DEPENDS iso
                USES_TERMINAL
                COMMENT "Boots JANUS aarch64 ISO via Limine UEFI"
            )

            add_custom_target(run-uefi
                COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
                COMMAND qemu-system-aarch64 
                    -M virt
                    -cpu cortex-a72
                    -m 256M
                    -device ramfb
                    -serial stdio
                    -bios ${AARCH64_UEFI_FIRMWARE}
                    -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    -boot d
                    ${QEMU_DEBUG_OPTS}
                    ${QEMU_DISPLAY_ARG}
                DEPENDS iso
                USES_TERMINAL
                COMMENT "Boots JANUS aarch64 ISO via Limine UEFI"
            )
        else()
            add_custom_target(run
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: aarch64 UEFI firmware not found."
                COMMAND ${CMAKE_COMMAND} -E echo "Install UEFI firmware:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S edk2-aarch64"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install qemu-efi-aarch64"
                COMMAND ${CMAKE_COMMAND} -E false
                USES_TERMINAL
                COMMENT "aarch64 UEFI firmware not found"
            )

            add_custom_target(run-uefi
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: aarch64 UEFI firmware not found."
                COMMAND ${CMAKE_COMMAND} -E echo "Install UEFI firmware:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S edk2-aarch64"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install qemu-efi-aarch64"
                COMMAND ${CMAKE_COMMAND} -E false
                USES_TERMINAL
                COMMENT "aarch64 UEFI firmware not found"
            )
        endif()

        # GRUB not available on aarch64
        add_custom_target(run-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: GRUB run target only available for x86_64"
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "GRUB run target only available for x86_64"
        )

        # Direct kernel boot (bypasses Limine - for debugging only)
        add_custom_target(run-elf
            COMMAND ${CMAKE_COMMAND} -E echo "WARNING: Direct kernel boot bypasses Limine bootloader."
            COMMAND ${CMAKE_COMMAND} -E echo "         The kernel will not receive Limine protocol data."
            COMMAND ${CMAKE_COMMAND} -E echo "         Use 'ninja run' for proper Limine boot."
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-aarch64
                -M virt
                -cpu cortex-a72
                -m 256M
                -serial stdio
                -kernel $<TARGET_FILE:kernel-limine.elf>
                ${QEMU_DEBUG_OPTS}
                ${QEMU_DISPLAY_ARG}
            DEPENDS kernel-limine.elf
            USES_TERMINAL
            COMMENT "Boots JANUS kernel directly (no Limine - limited functionality)"
        )
    else()
        # x86_64: Original implementation
        # QEMU debug logging options - logs to build directory
        set(QEMU_DEBUG_LOG "${CMAKE_BINARY_DIR}/qemu-debug.log")
        set(QEMU_DEBUG_OPTS -d cpu_reset,guest_errors,unimp -D ${QEMU_DEBUG_LOG})

        # Run Limine ISO
        add_custom_target(run
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME} 
                -boot d -serial stdio -m 256M ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
            DEPENDS iso
            USES_TERMINAL
            COMMENT "Boots JANUS Limine ISO (select boot protocol from menu)"
        )

        # Run GRUB ISO
        add_custom_target(run-grub
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME}
                -boot d -serial stdio -m 256M ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
            DEPENDS iso-grub
            USES_TERMINAL
            COMMENT "Boots JANUS GRUB ISO (Multiboot2 - educational)"
        )

        # Direct kernel load (not supported - QEMU requires Multiboot1 for -kernel, and our Multiboot2 kernel is not compatible)
        add_custom_target(run-elf
            COMMAND ${CMAKE_COMMAND} -E echo "Note: QEMU -kernel only supports Multiboot1, not Multiboot2."
            COMMAND ${CMAKE_COMMAND} -E echo "Use 'ninja run' to boot via ISO instead."
            COMMAND ${CMAKE_COMMAND} -E false
            DEPENDS kernel-multiboot2.elf
            USES_TERMINAL
            COMMENT "Direct kernel boot not supported (QEMU requires Multiboot1)"
        )

        # UEFI boot
        add_custom_target(run-uefi
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                -boot d -serial stdio -m 256M ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
                -bios /usr/share/OVMF/OVMF_CODE.fd
            DEPENDS iso
            USES_TERMINAL
            COMMENT "Boots JANUS ISO in UEFI mode (requires OVMF)"
        )
    endif()
endfunction()

function(janus_add_debug_targets)
    set(JANUS_ISO_NAME "janus_${JANUS_TARGET_ARCH}.iso")
    set(JANUS_ISO_GRUB_NAME "janus_${JANUS_TARGET_ARCH}_grub.iso")

    # Check for appropriate QEMU binary
    if(NOT JANUS_QEMU_FOUND)
        add_custom_target(debug
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        add_custom_target(debug-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        add_custom_target(debug-elf
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: QEMU not installed."
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "QEMU not installed"
        )
        return()
    endif()

    # QEMU debug logging for debug targets - more verbose than run targets
    set(QEMU_DEBUG_LOG "${CMAKE_BINARY_DIR}/qemu-debug.log")
    # Debug targets include 'int' for exception/interrupt tracing
    set(QEMU_DEBUG_OPTS_VERBOSE -d cpu_reset,guest_errors,unimp,int -D ${QEMU_DEBUG_LOG})

    if(JANUS_TARGET_ARCH STREQUAL "aarch64")
        # aarch64: Debug with UEFI ISO
        # Find UEFI firmware for aarch64
        set(AARCH64_UEFI_FIRMWARE "")
        if(EXISTS "/usr/share/AAVMF/AAVMF_CODE.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/AAVMF/AAVMF_CODE.fd")
        elseif(EXISTS "/usr/share/edk2/aarch64/QEMU_EFI.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/edk2/aarch64/QEMU_EFI.fd")
        elseif(EXISTS "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd")
            set(AARCH64_UEFI_FIRMWARE "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd")
        endif()

        if(AARCH64_UEFI_FIRMWARE)
            add_custom_target(debug
                COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log : ${QEMU_DEBUG_LOG}"
                COMMAND qemu-system-aarch64
                    -M virt
                    -cpu cortex-a72
                    -m 256M
                    -device ramfb
                    -serial stdio
                    -bios ${AARCH64_UEFI_FIRMWARE}
                    -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                    -boot d
                    -s -S
                    ${QEMU_DEBUG_OPTS_VERBOSE}
                    ${QEMU_DISPLAY_ARG}
                DEPENDS iso
                USES_TERMINAL
                COMMENT "Boots JANUS aarch64 ISO with GDB server on :1234"
            )
        else()
            add_custom_target(debug
                COMMAND ${CMAKE_COMMAND} -E echo "ERROR: aarch64 UEFI firmware not found."
                COMMAND ${CMAKE_COMMAND} -E echo "Install UEFI firmware:"
                COMMAND ${CMAKE_COMMAND} -E echo "  Arch: sudo pacman -S edk2-aarch64"
                COMMAND ${CMAKE_COMMAND} -E echo "  Debian/Ubuntu: sudo apt install qemu-efi-aarch64"
                COMMAND ${CMAKE_COMMAND} -E false
                USES_TERMINAL
                COMMENT "aarch64 UEFI firmware not found"
            )
        endif()

        # GRUB not available on aarch64
        add_custom_target(debug-grub
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: GRUB debug target only available for x86_64"
            COMMAND ${CMAKE_COMMAND} -E false
            USES_TERMINAL
            COMMENT "GRUB debug target only available for x86_64"
        )

        # Direct kernel debug (bypasses Limine)
        add_custom_target(debug-elf
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log [verbose]: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-aarch64
                -M virt
                -cpu cortex-a72
                -m 256M
                -serial stdio
                -kernel $<TARGET_FILE:kernel-limine.elf>
                -s -S
                ${QEMU_DEBUG_OPTS_VERBOSE}
                ${QEMU_DISPLAY_ARG}
            DEPENDS kernel-limine.elf
            USES_TERMINAL
            COMMENT "Debug JANUS kernel directly (no Limine - limited functionality)"
        )
    else()
        # x86_64: Original implementation
        # Debug Limine ISO
        add_custom_target(debug
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
                -boot d -serial stdio -m 256M -s -S ${QEMU_DEBUG_OPTS_VERBOSE} ${QEMU_DISPLAY_ARG}
            DEPENDS iso
            USES_TERMINAL
            COMMENT "Boots JANUS Limine ISO with GDB server on :1234"
        )

        # Debug GRUB ISO
        add_custom_target(debug-grub
            COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log [verbose]: ${QEMU_DEBUG_LOG}"
            COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_GRUB_NAME}
                -boot d -serial stdio -m 256M -s -S ${QEMU_DEBUG_OPTS_VERBOSE} ${QEMU_DISPLAY_ARG}
            DEPENDS iso-grub
            USES_TERMINAL
            COMMENT "Boots JANUS GRUB ISO with GDB server on :1234 (Multiboot2 - educational)"
        )

        # Direct kernel debug (not supported)
        add_custom_target(debug-elf
            COMMAND ${CMAKE_COMMAND} -E echo "Note: QEMU -kernel only supports Multiboot1, not Multiboot2."
            COMMAND ${CMAKE_COMMAND} -E echo "Use 'ninja debug' to boot via ISO instead."
            COMMAND ${CMAKE_COMMAND} -E false
            DEPENDS kernel.elf
            USES_TERMINAL
            COMMENT "Direct kernel debug not supported (QEMU requires Multiboot1)"
        )
    endif()
endfunction()

function(janus_setup_targets)
    janus_check_required_tools()
    janus_find_limine()
    janus_configure_qemu_display()
    janus_add_iso_targets()
    janus_add_run_targets()
    janus_add_debug_targets()
endfunction()
