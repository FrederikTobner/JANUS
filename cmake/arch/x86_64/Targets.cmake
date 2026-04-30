#[[
    Targets.cmake — x86_64 arch layer

    Sets QEMU configuration variables and defines arch-specific special targets.
    Included by the root Targets.cmake orchestrator.

    Must define:
      janus_configure_qemu_x86_64()        — sets QEMU variables in PARENT_SCOPE
      janus_add_special_targets_x86_64()   — creates run-elf, run-uefi, debug-elf
]]

function(janus_configure_qemu_x86_64)
    set(QEMU_BINARY qemu-system-x86_64 PARENT_SCOPE)
    set(QEMU_MACHINE_FLAGS "" PARENT_SCOPE)
    set(QEMU_INSTALL_HINT
        "Install QEMU:"
        "  Arch: sudo pacman -S qemu-system-x86"
        "  Debian/Ubuntu: sudo apt install qemu-system-x86"
        PARENT_SCOPE)

    set(QEMU_DEBUG_LOG "${CMAKE_BINARY_DIR}/qemu-debug.log" PARENT_SCOPE)
    set(QEMU_DEBUG_OPTS
        -d cpu_reset,guest_errors,unimp -D "${CMAKE_BINARY_DIR}/qemu-debug.log"
        PARENT_SCOPE)
    set(QEMU_DEBUG_OPTS_VERBOSE
        -d cpu_reset,guest_errors,unimp,int -D "${CMAKE_BINARY_DIR}/qemu-debug.log"
        PARENT_SCOPE)
endfunction()

function(janus_add_special_targets_x86_64)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(run-elf "QEMU not installed." ${QEMU_INSTALL_HINT})
        janus_error_target(run-uefi "QEMU not installed." ${QEMU_INSTALL_HINT})
        janus_error_target(debug-elf "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    # QEMU -kernel requires Multiboot1 headers, which we don't provide
    janus_error_target(run-elf
        "QEMU -kernel only supports Multiboot1, not Multiboot2."
        "Use 'ninja run-limine' to boot via ISO instead.")
    janus_error_target(debug-elf
        "QEMU -kernel only supports Multiboot1, not Multiboot2."
        "Use 'ninja debug-limine' instead.")

    # UEFI boot via OVMF
    add_custom_target(run-uefi
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY}
            -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
            -boot d -serial stdio -m 256M
            -bios /usr/share/OVMF/OVMF_CODE.fd
            ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
        DEPENDS iso-limine
        USES_TERMINAL
        COMMENT "Boots JANUS ISO in UEFI mode (requires OVMF)"
    )
endfunction()
