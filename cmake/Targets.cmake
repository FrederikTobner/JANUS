#[[
    Targets.cmake — Root orchestrator

    Three-layer target system mirroring the kernel source structure:
      cmake/Targets.cmake                      ← this file: orchestration + shared helpers
      cmake/arch/<arch>/Targets.cmake          ← QEMU config + special targets (run-elf, etc.)
      cmake/boot/<proto>/Targets.cmake         ← iso/run/debug per boot protocol

    Protocols are discovered by convention: a protocol exists if and only if
    cmake/boot/<proto>/Targets.cmake is present. JANUS_BOOT_PROTOCOLS
    (set per-arch in cmake/arch/<arch>/platform/CompilerFlags.cmake) selects which
    protocols to enable. Validation ensures consistency.

    Limine is fetched automatically via FetchContent.
]]

include(FetchContent)
FetchContent_Declare(
    limine
    GIT_REPOSITORY https://github.com/limine-bootloader/limine.git
    GIT_TAG        v8.x-binary
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(limine)

# Discover all known protocols from cmake/boot/*/Targets.cmake
file(GLOB _proto_modules
     "${CMAKE_SOURCE_DIR}/cmake/boot/*/Targets.cmake")

set(JANUS_ALL_PROTOCOLS "")
foreach(_mod IN LISTS _proto_modules)
    get_filename_component(_dir "${_mod}" DIRECTORY)
    get_filename_component(_proto "${_dir}" NAME)
    list(APPEND JANUS_ALL_PROTOCOLS "${_proto}")
endforeach()

# Validate: every arch-selected protocol must have a module file
foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
    if(NOT _proto IN_LIST JANUS_ALL_PROTOCOLS)
        message(FATAL_ERROR
            "Protocol '${_proto}' listed in JANUS_BOOT_PROTOCOLS but "
            "cmake/boot/${_proto}/Targets.cmake does not exist.")
    endif()
endforeach()

# Note protocols with modules but not enabled for this arch
foreach(_proto IN LISTS JANUS_ALL_PROTOCOLS)
    if(NOT _proto IN_LIST JANUS_BOOT_PROTOCOLS)
        message(STATUS "Protocol '${_proto}' has a module but is not "
            "enabled for ${JANUS_TARGET_ARCH}.")
    endif()
endforeach()

# Create a target that prints an error message and fails.
function(janus_error_target TARGET_NAME ERROR_MSG)
    set(_cmds COMMAND ${CMAKE_COMMAND} -E echo "ERROR: ${ERROR_MSG}")
    foreach(_hint IN LISTS ARGN)
        list(APPEND _cmds COMMAND ${CMAKE_COMMAND} -E echo "${_hint}")
    endforeach()
    list(APPEND _cmds COMMAND ${CMAKE_COMMAND} -E false)
    add_custom_target(${TARGET_NAME} ${_cmds}
        USES_TERMINAL
        COMMENT "${ERROR_MSG}")
endfunction()

# Check for required external tools and provide helpful error messages.
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

    # QEMU (emulation) — binary name comes from arch layer
    find_program(QEMU_EXECUTABLE ${QEMU_BINARY})
    if(NOT QEMU_EXECUTABLE)
        list(APPEND MISSING_TOOLS "${QEMU_BINARY}")
        string(REPLACE ";" "\n" _hint "${QEMU_INSTALL_HINT}")
        list(APPEND INSTALL_HINTS "  ${QEMU_BINARY}:\n${_hint}")
    endif()

    # grub-mkrescue (GRUB ISO creation)
    find_program(GRUB_MKRESCUE_EXECUTABLE grub-mkrescue)
    if(NOT GRUB_MKRESCUE_EXECUTABLE)
        list(APPEND MISSING_TOOLS "grub-mkrescue")
        list(APPEND INSTALL_HINTS "  grub-mkrescue:\n    Arch: sudo pacman -S grub\n    Debian/Ubuntu: sudo apt install grub-pc-bin grub-common xorriso mtools")
    endif()

    # Limine is fetched automatically via FetchContent (no system install needed)

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
endfunction()

# Set up Limine data directory and CLI tool from FetchContent.
function(janus_find_limine)
    # Limine fetched via FetchContent — data files live at the source root
    set(LIMINE_DATA_DIR "${limine_SOURCE_DIR}" PARENT_SCOPE)

    # Build the Limine CLI tool from source (needed for bios-install on x86_64).
    # The binary branch ships limine.c + a Makefile that compiles it with the host CC.
    if(JANUS_TARGET_ARCH STREQUAL "x86_64")
        set(LIMINE_EXECUTABLE "${limine_SOURCE_DIR}/limine" PARENT_SCOPE)
        add_custom_command(
            OUTPUT "${limine_SOURCE_DIR}/limine"
            COMMAND make -C "${limine_SOURCE_DIR}"
            COMMENT "Building Limine CLI tool"
        )
        add_custom_target(limine-cli DEPENDS "${limine_SOURCE_DIR}/limine")
    endif()
endfunction()

# Detect QEMU display backend for tiling WMs.
function(janus_configure_qemu_display)
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

function(janus_add_targets)
    set(JANUS_ISO_NAME "janus_${JANUS_TARGET_ARCH}.iso")

    # Configure boot protocol templates
    if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/limine/limine.conf.in
            ${CMAKE_BINARY_DIR}/limine.conf
            @ONLY
        )
    endif()
    if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
            ${CMAKE_BINARY_DIR}/grub.cfg
            @ONLY
        )
    endif()

    # Include and call per-protocol modules for enabled protocols
    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        include("${CMAKE_SOURCE_DIR}/cmake/boot/${_proto}/Targets.cmake")
        cmake_language(CALL janus_iso_${_proto})
        cmake_language(CALL janus_run_${_proto})
        cmake_language(CALL janus_debug_${_proto})
    endforeach()

    # Error stubs for protocols not supported on this architecture
    foreach(_proto IN LISTS JANUS_ALL_PROTOCOLS)
        if(NOT _proto IN_LIST JANUS_BOOT_PROTOCOLS)
            janus_error_target(iso-${_proto}
                "${_proto} not supported on ${JANUS_TARGET_ARCH}.")
            janus_error_target(run-${_proto}
                "${_proto} not supported on ${JANUS_TARGET_ARCH}.")
            janus_error_target(debug-${_proto}
                "${_proto} not supported on ${JANUS_TARGET_ARCH}.")
        endif()
    endforeach()

    # Umbrella iso target
    set(_iso_deps)
    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        list(APPEND _iso_deps iso-${_proto})
    endforeach()
    add_custom_target(iso DEPENDS ${_iso_deps})

    # Arch-specific special targets (run-elf, run-uefi, debug-elf)
    cmake_language(CALL janus_add_special_targets_${JANUS_TARGET_ARCH})
endfunction()

function(janus_setup_targets)
    # 1. Include arch layer — sets QEMU variables
    include("${CMAKE_SOURCE_DIR}/cmake/arch/${JANUS_TARGET_ARCH}/Targets.cmake")
    cmake_language(CALL janus_configure_qemu_${JANUS_TARGET_ARCH})

    # 2. Check external tools (uses QEMU_BINARY from arch layer)
    janus_check_required_tools()

    # 3. Set up Limine FetchContent paths
    janus_find_limine()

    # 4. Detect QEMU display backend
    janus_configure_qemu_display()

    # 5. Create all targets (protocol + arch-specific)
    janus_add_targets()
endfunction()

function(janus_print_targets)

    message(STATUS "Available targets:")
    # Compute the widest command column so every " - " separator aligns.
    # Column width = length of the longest "ninja <cmd>" string.
    set(_max_col 5) # bare "ninja"
    foreach(_cmd IN ITEMS "iso" "run-uefi" "run-elf")
        string(LENGTH "${_cmd}" _len)
        math(EXPR _col "6 + ${_len}")
        if(_col GREATER _max_col)
            set(_max_col ${_col})
        endif()
    endforeach()
    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        foreach(_pfx IN ITEMS "iso-" "run-" "debug-")
            string(LENGTH "${_pfx}${_proto}" _len)
            math(EXPR _col "6 + ${_len}")
            if(_col GREATER _max_col)
                set(_max_col ${_col})
            endif()
        endforeach()
    endforeach()
    # Print one aligned target line: "  ninja [<cmd>]<pad> - <desc>"
    macro(_janus_msg _cmd _desc)
        if("${_cmd}" STREQUAL "")
            math(EXPR _pad "${_max_col} - 5 + 1")
            string(REPEAT " " ${_pad} _sp)
            message(STATUS "  ninja${_sp}- ${_desc}")
        else()
            string(LENGTH "${_cmd}" _clen)
            math(EXPR _pad "${_max_col} - 6 - ${_clen} + 1")
            string(REPEAT " " ${_pad} _sp)
            message(STATUS "  ninja ${_cmd}${_sp}- ${_desc}")
        endif()
    endmacro()
    _janus_msg("" "Build kernel ELFs")
    _janus_msg("iso" "Create all ISOs for this platform")
    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        _janus_msg("iso-${_proto}" "Create ISO for the ${_proto} boot protocol")
        _janus_msg("run-${_proto}" "Run ISO for the ${_proto} boot protocol")
        _janus_msg("debug-${_proto}" "Debug ISO for the ${_proto} boot protocol")
    endforeach()
    _janus_msg("run-uefi" "Boot ISO in UEFI mode")
    _janus_msg("run-elf" "Direct kernel boot (debug)")
    message(STATUS "================================================================================")
endfunction()
