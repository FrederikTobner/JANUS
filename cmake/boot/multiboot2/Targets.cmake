#[[
    Targets.cmake — Multiboot2 boot protocol targets

    Provides:
      janus_iso_multiboot2()    — creates the iso-multiboot2 custom target
      janus_run_multiboot2()    — creates the run-multiboot2 custom target
      janus_debug_multiboot2()  — creates the debug-multiboot2 custom target

    Included by the root Targets.cmake orchestrator.
    Only supported on x86_64 (GRUB with Multiboot2).
]]

function(janus_iso_multiboot2)
    if(NOT JANUS_GRUB_MKRESCUE_FOUND)
        janus_error_target(iso-multiboot2 "grub-mkrescue not installed."
            "Install GRUB tools:"
            "  Arch: sudo pacman -S grub"
            "  Debian/Ubuntu: sudo apt install grub-pc-bin grub-common xorriso mtools")
        return()
    endif()

    set(_grub_iso "janus_${JANUS_TARGET_ARCH}_grub.iso")

    add_custom_target(iso-multiboot2
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/iso-grub
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso-grub/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel-multiboot2.elf>
                ${CMAKE_BINARY_DIR}/iso-grub/boot/kernel.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg
                ${CMAKE_BINARY_DIR}/iso-grub/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/${_grub_iso}
                ${CMAKE_BINARY_DIR}/iso-grub/
        DEPENDS kernel-multiboot2.elf ${CMAKE_BINARY_DIR}/grub.cfg
        COMMENT "Creates a bootable ISO image (GRUB/Multiboot2)"
    )
endfunction()

function(janus_run_multiboot2)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(run-multiboot2 "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    set(_grub_iso "janus_${JANUS_TARGET_ARCH}_grub.iso")

    add_custom_target(run-multiboot2
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY} ${QEMU_MACHINE_FLAGS}
            -cdrom ${CMAKE_BINARY_DIR}/${_grub_iso}
            -boot d -serial stdio -m 256M
            ${QEMU_DEBUG_OPTS} ${QEMU_DISPLAY_ARG}
        DEPENDS iso-multiboot2
        USES_TERMINAL
        COMMENT "Boots JANUS Multiboot2 ISO (GRUB)"
    )
endfunction()

function(janus_debug_multiboot2)
    if(NOT JANUS_QEMU_FOUND)
        janus_error_target(debug-multiboot2 "QEMU not installed." ${QEMU_INSTALL_HINT})
        return()
    endif()

    set(_grub_iso "janus_${JANUS_TARGET_ARCH}_grub.iso")

    add_custom_target(debug-multiboot2
        COMMAND ${CMAKE_COMMAND} -E echo "QEMU debug log [verbose]: ${QEMU_DEBUG_LOG}"
        COMMAND ${QEMU_BINARY} ${QEMU_MACHINE_FLAGS}
            -cdrom ${CMAKE_BINARY_DIR}/${_grub_iso}
            -boot d -serial stdio -m 256M -s -S
            ${QEMU_DEBUG_OPTS_VERBOSE} ${QEMU_DISPLAY_ARG}
        DEPENDS iso-multiboot2
        USES_TERMINAL
        COMMENT "Boots JANUS Multiboot2 ISO with GDB server on :1234 (GRUB)"
    )
endfunction()
