# Toolchain: aarch64 cross-compilation with Clang
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(JANUS_TARGET_ARCH "aarch64" CACHE STRING "Target architecture")

# Clang as multi-target compiler
find_program(CLANG_EXECUTABLE clang REQUIRED)
set(CMAKE_C_COMPILER "${CLANG_EXECUTABLE}")
set(CMAKE_ASM_COMPILER "${CLANG_EXECUTABLE}")

# Tell Clang the target for C and ASM
set(CMAKE_C_COMPILER_TARGET aarch64-elf)
set(CMAKE_ASM_COMPILER_TARGET aarch64-elf)

# Use LLVM binutils
find_program(CMAKE_AR llvm-ar REQUIRED)
find_program(CMAKE_RANLIB llvm-ranlib REQUIRED)

# Cross-linker and objcopy (GNU ld for aarch64 — needed for linker scripts)
find_program(CMAKE_LINKER aarch64-linux-gnu-ld REQUIRED)
find_program(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy REQUIRED)

# Tell Clang which linker to use when it drives linking
set(CMAKE_EXE_LINKER_FLAGS_INIT "--target=aarch64-elf -fuse-ld=${CMAKE_LINKER}")

# Prevent CMake from testing the compiler with a program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
