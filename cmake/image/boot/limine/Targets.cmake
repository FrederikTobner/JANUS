#[[
    Targets.cmake — Limine boot protocol targets

    Provides:
      janus_iso_limine()    — creates the iso-limine custom target
      janus_run_limine()    — creates the run-limine custom target
      janus_debug_limine()  — creates the debug-limine custom target

    Included by the root Targets.cmake orchestrator.
    On x86_64 produces a BIOS + UEFI hybrid ISO; on aarch64 UEFI-only.
]]

function(janus_iso_limine)
    if(NOT JANUS_XORRISO_FOUND)
        janus_error_target(iso-limine "xorriso not installed."
            "Install xorriso:"
            "  Arch: sudo pacman -S libisoburn"
            "  Debian/Ubuntu: sudo apt install xorriso")
        return()
    endif()

    # Common staging: kernel, limine.conf, UEFI CD image
    set(_cmds
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/limine
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-limine.elf>
                ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/limine.conf
                ${CMAKE_BINARY_DIR}/iso/boot/limine.conf
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-uefi-cd.bin
                ${CMAKE_BINARY_DIR}/iso/boot/limine/
    )
    set(_deps kernel-limine.elf ${CMAKE_BINARY_DIR}/limine.conf)

    if(JANUS_TARGET_ARCH STREQUAL "x86_64")
        # BIOS + UEFI hybrid ISO with bios-install
        list(APPEND _cmds
            COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios.sys
                    ${CMAKE_BINARY_DIR}/iso/boot/limine/
            COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/limine-bios-cd.bin
                    ${CMAKE_BINARY_DIR}/iso/boot/limine/
            COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/BOOTX64.EFI
                    ${CMAKE_BINARY_DIR}/iso/EFI/BOOT/
            COMMAND xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin
                -no-emul-boot -boot-load-size 4 -boot-info-table
                --efi-boot boot/limine/limine-uefi-cd.bin
                -efi-boot-part --efi-boot-image --protective-msdos-label
                ${CMAKE_BINARY_DIR}/iso -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            COMMAND ${LIMINE_EXECUTABLE} bios-install
                    ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
        )
        list(APPEND _deps limine-cli)
    else()
        # aarch64: UEFI-only ISO
        list(APPEND _cmds
            COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DATA_DIR}/BOOTAA64.EFI
                    ${CMAKE_BINARY_DIR}/iso/EFI/BOOT/
            COMMAND xorriso -as mkisofs
                --efi-boot boot/limine/limine-uefi-cd.bin
                -efi-boot-part --efi-boot-image --protective-msdos-label
                ${CMAKE_BINARY_DIR}/iso -o ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
        )
    endif()

    add_custom_target(iso-limine ${_cmds}
        DEPENDS ${_deps}
        COMMENT "Creates a bootable ISO image (Limine)"
    )
endfunction()

function(janus_run_limine)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(run-limine "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    set(_qemu_extra "")
    if(JANUS_TARGET_ARCH STREQUAL "aarch64")
        if(NOT AARCH64_UEFI_FIRMWARE)
            janus_error_target(run-limine "aarch64 UEFI firmware not found."
                ${UEFI_FIRMWARE_INSTALL_HINT})
            return()
        endif()
        set(_qemu_extra -bios ${AARCH64_UEFI_FIRMWARE})
    endif()

    add_custom_target(run-limine
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY} ${QEMU_MACHINE_FLAGS}
            -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            -boot d -serial stdio -m 256M
            ${_qemu_extra} ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
        DEPENDS iso-limine
        USES_TERMINAL
        COMMENT "Boots JANUS ISO via Limine"
    )
endfunction()

function(janus_debug_limine)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(debug-limine "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    set(_qemu_extra "")
    if(JANUS_TARGET_ARCH STREQUAL "aarch64")
        if(NOT AARCH64_UEFI_FIRMWARE)
            janus_error_target(debug-limine "aarch64 UEFI firmware not found."
                ${UEFI_FIRMWARE_INSTALL_HINT})
            return()
        endif()
        set(_qemu_extra -bios ${AARCH64_UEFI_FIRMWARE})
    endif()

    add_custom_target(debug-limine
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log [verbose]: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY} ${QEMU_MACHINE_FLAGS}
            -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            -boot d -serial stdio -m 256M -s -S
            ${_qemu_extra} ${QEMU_DEBUG_OPTS_VERBOSE} ${QEMU_DISPLAY_ARG}
        DEPENDS iso-limine
        USES_TERMINAL
        COMMENT "Boots JANUS ISO via Limine with GDB server on :1234"
    )
endfunction()
