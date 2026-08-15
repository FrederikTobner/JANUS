#[[ 
    CompilerFlags.cmake - JANUS Kernel ABI Flags (aarch64)

    Kernel-only compiler flags for aarch64: general-registers-only (no
    NEON/FP registers, since kernel code — especially interrupt/exception
    handlers — can't safely save/restore them without extra work).
    Compiler and linker are set by the toolchain file. Boot-protocol support
    for this arch is a separate, shared fact — see
    cmake/arch/aarch64/BootProtocols.cmake.
]]

include_guard(GLOBAL)

if(JANUS_COMPILER_CLANG)
    set(JANUS_ARCH_FLAGS
        --target=aarch64-elf
        -march=armv8-a
        -mgeneral-regs-only
    )
elseif(JANUS_COMPILER_GCC)
    set(JANUS_ARCH_FLAGS
        -march=armv8-a
        -mgeneral-regs-only
    )
else()
    message(FATAL_ERROR "Unsupported compiler for aarch64: ${CMAKE_C_COMPILER_ID}")
endif()
