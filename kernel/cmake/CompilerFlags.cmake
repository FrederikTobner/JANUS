#[[ 
    CompilerFlags.cmake - JANUS Kernel Compiler Flags
    Kernel ABI flags (from kernel/cmake/arch/<arch>/) and common freestanding kernel compiler flags.

    Expects: JANUS_COMPILER_CLANG or JANUS_COMPILER_GCC set (by platform/Detection.cmake)
    Expects: JANUS_TARGET_ARCH set (by toolchain file)
]]

include_guard(GLOBAL)

# Include kernel ABI flags for this architecture (sets JANUS_ARCH_FLAGS)
include(${CMAKE_CURRENT_LIST_DIR}/arch/${JANUS_TARGET_ARCH}/CompilerFlags.cmake)

set(JANUS_COMPILE_OPTIONS_COMMON
    ${JANUS_ARCH_FLAGS}
    -nostdlib
    -ffreestanding
    -fno-builtin
    -fno-stack-protector
    -Wall
    -Wextra
    -Werror
    # Additional warnings for stricter code quality
    -Wconversion
    -Wimplicit
    -Wcast-qual
    -Wpointer-arith
)

set(JANUS_COMPILE_OPTIONS_DEBUG
    -g3
    -gdwarf-4
    -O0
    -DDEBUG
)

set(JANUS_COMPILE_OPTIONS_RELEASE
    -O2
    -DNDEBUG
)

set(JANUS_COMPILE_OPTIONS_MINSIZEREL
    -Os
    -DNDEBUG
)

function(janus_apply_compile_flags TARGET)
    target_compile_options(${TARGET} PRIVATE ${JANUS_COMPILE_OPTIONS_COMMON})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET} PRIVATE ${JANUS_COMPILE_OPTIONS_DEBUG})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${TARGET} PRIVATE ${JANUS_COMPILE_OPTIONS_RELEASE})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${TARGET} PRIVATE ${JANUS_COMPILE_OPTIONS_MINSIZEREL})
    endif()
endfunction()
