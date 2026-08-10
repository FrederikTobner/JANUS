#[[ 
    CompilerFlags.cmake - JANUS Compiler Flags
    Architecture-specific flags (from arch layer) + common kernel compiler flags

    Expects: JANUS_COMPILER_CLANG or JANUS_COMPILER_GCC set (by platform/Detection.cmake)
    Expects: JANUS_TARGET_ARCH set (by toolchain file)
]]

include_guard(GLOBAL)

# Include architecture-specific flags (sets JANUS_ARCH_FLAGS, JANUS_BOOT_PROTOCOLS)
include(${CMAKE_SOURCE_DIR}/cmake/arch/${JANUS_TARGET_ARCH}/platform/CompilerFlags.cmake)

# Common compiler flags for all kernel code
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

# Debug-specific flags
set(JANUS_COMPILE_OPTIONS_DEBUG
    -g3
    -gdwarf-4
    -O0
    -DDEBUG
)

# Release-specific flags
set(JANUS_COMPILE_OPTIONS_RELEASE
    -O2
    -DNDEBUG
)

# MinSizeRel flags (optimize for size)
set(JANUS_COMPILE_OPTIONS_MINSIZEREL
    -Os
    -DNDEBUG
)

# Apply all compile flags (common + build-type) to a target.
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
