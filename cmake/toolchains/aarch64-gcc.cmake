# Toolchain: aarch64 cross-compilation with GCC
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(JANUS_TARGET_ARCH "aarch64" CACHE STRING "Target architecture")

# Cross-compiler
find_program(AARCH64_GCC aarch64-linux-gnu-gcc REQUIRED)
set(CMAKE_C_COMPILER "${AARCH64_GCC}")
set(CMAKE_ASM_COMPILER "${AARCH64_GCC}")

# Cross-binutils
find_program(CMAKE_AR aarch64-linux-gnu-ar REQUIRED)
find_program(CMAKE_RANLIB aarch64-linux-gnu-ranlib REQUIRED)
find_program(CMAKE_LINKER aarch64-linux-gnu-ld REQUIRED)
find_program(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy REQUIRED)

# Prevent CMake from testing the compiler with a full program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
