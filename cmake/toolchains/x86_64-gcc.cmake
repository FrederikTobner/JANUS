# Toolchain: x86_64 with GCC (native on x86_64 host)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")

# Use system GCC (or cross-compiler if on non-x86 host)
if(NOT CMAKE_C_COMPILER)
    find_program(X86_64_GCC gcc REQUIRED)
    set(CMAKE_C_COMPILER "${X86_64_GCC}")
endif()

# Prevent CMake from testing the compiler with a full program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
