#[[
    Targets.cmake — aarch64 arch layer

    Sets QEMU configuration variables and defines arch-specific special targets.
    Included by the root Targets.cmake orchestrator.

    Must define:
      janus_configure_qemu_aarch64()        — sets QEMU variables in PARENT_SCOPE
      janus_add_special_targets_aarch64()   — creates run-elf, run-uefi, debug-elf
]]

function(janus_configure_qemu_aarch64)
    set(QEMU_BINARY qemu-system-aarch64 PARENT_SCOPE)
    set(QEMU_MACHINE_FLAGS -M virt -cpu cortex-a72 -device ramfb PARENT_SCOPE)
    set(QEMU_INSTALL_HINT
        "Install QEMU:"
        "  Arch: sudo pacman -S qemu-system-aarch64"
        "  Debian/Ubuntu: sudo apt install qemu-system-arm"
        PARENT_SCOPE)

    set(QEMU_DEBUG_LOG "${CMAKE_BINARY_DIR}/qemu-debug.log" PARENT_SCOPE)
    set(QEMU_DEBUG_OPTS
        -d cpu_reset,guest_errors,unimp -D "${CMAKE_BINARY_DIR}/qemu-debug.log"
        PARENT_SCOPE)
    set(QEMU_DEBUG_OPTS_VERBOSE
        -d cpu_reset,guest_errors,unimp,int -D "${CMAKE_BINARY_DIR}/qemu-debug.log"
        PARENT_SCOPE)

    # Find UEFI firmware (searched once, used by run/debug targets)
    set(AARCH64_UEFI_FIRMWARE "" PARENT_SCOPE)
    set(UEFI_FIRMWARE_INSTALL_HINT
        "Install UEFI firmware:"
        "  Arch: sudo pacman -S edk2-aarch64"
        "  Debian/Ubuntu: sudo apt install qemu-efi-aarch64"
        PARENT_SCOPE)
    foreach(_fw
        "/usr/share/AAVMF/AAVMF_CODE.fd"
        "/usr/share/edk2/aarch64/QEMU_EFI.fd"
        "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd")
        if(EXISTS "${_fw}")
            set(AARCH64_UEFI_FIRMWARE "${_fw}" PARENT_SCOPE)
            break()
        endif()
    endforeach()
endfunction()

function(janus_add_special_targets_aarch64)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(run-elf "QEMU not installed." ${QEMU_INSTALL_HINT})
        janus_error_target(run-uefi "QEMU not installed." ${QEMU_INSTALL_HINT})
        janus_error_target(debug-elf "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    # Direct kernel boot (bypasses Limine — for debugging only)
    add_custom_target(run-elf
        COMMAND ${CMAKE_COMMAND} -E echo
            "WARNING: Direct kernel boot bypasses Limine bootloader."
        COMMAND ${CMAKE_COMMAND} -E echo
            "         The kernel will not receive Limine protocol data."
        COMMAND ${CMAKE_COMMAND} -E echo
            "         Use 'ninja run-limine' for proper Limine boot."
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY}
            -M virt -cpu cortex-a72
            -m 256M -serial stdio
            -kernel $<TARGET_FILE:kernel-limine.elf>
            ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
        DEPENDS kernel-limine.elf
        USES_TERMINAL
        COMMENT "Boots kernel directly (no Limine — limited)"
    )

    # Direct kernel debug (bypasses Limine)
    add_custom_target(debug-elf
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log [verbose]: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY}
            -M virt -cpu cortex-a72
            -m 256M -serial stdio
            -kernel $<TARGET_FILE:kernel-limine.elf>
            -s -S ${QEMU_DEBUG_OPTS_VERBOSE} ${QEMU_DISPLAY_ARG}
        DEPENDS kernel-limine.elf
        USES_TERMINAL
        COMMENT "Debug kernel directly with GDB (no Limine)"
    )

    # run-uefi is the same as run-limine on aarch64 (always UEFI)
    add_custom_target(run-uefi DEPENDS run-limine)
endfunction()
