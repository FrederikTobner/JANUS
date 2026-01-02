cmake_minimum_required(VERSION 3.20)
project(TinyOS VERSION 0.1.0 LANGUAGES C ASM_NASM)

# Print build configuration
message(STATUS "========================================")
message(STATUS "TinyOS Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER} (version ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "Assembler: ${CMAKE_ASM_NASM_COMPILER}")
message(STATUS "========================================")
message(STATUS "Available targets:")
message(STATUS "  ninja           - Build kernel.elf")
message(STATUS "  ninja iso       - Create bootable ISO")
message(STATUS "  ninja run       - Build and run in QEMU")
message(STATUS "  ninja debug     - Build and run with debugger")
message(STATUS "========================================")

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # Allow GNU extensions

# Include TinyOS platform detection and helper functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(TinyOSPlatform)
include(TinyOSHelpers)

# Generate build timestamp
string(TIMESTAMP TINYOS_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP TINYOS_BUILD_TIME "%H:%M:%S")


# Copy compile_commands.json to project root for clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

add_subdirectory(kernel)

# ISO creation target
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso ${CMAKE_BINARY_DIR}/iso/
    DEPENDS kernel.elf ${CMAKE_BINARY_DIR}/grub.cfg
    COMMENT "Creating bootable ISO image"
)

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
            message(WARNING "Neither SDL2 nor GTK3 development libraries found. QEMU display argument will not be set. QEMU may not show a window under i3.")
        else()
            set(QEMU_DISPLAY_ARG -display gtk)
        endif()
    else()
        message(WARNING "Neither SDL2 nor GTK3 development libraries found. QEMU display argument will not be set. QEMU may not show a window under i3.")
    endif()
else()
    set(QEMU_DISPLAY_ARG)
endif()

# Custom target to run TinyOS in QEMU
add_custom_target(run
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Running TinyOS in QEMU"
)

# Custom target to run TinyOS in QEMU with GDB stub
add_custom_target(debug
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio -s -S ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
)

