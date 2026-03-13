# Toolchain: x86_64 with Clang
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")

# Clang as multi-target compiler
find_program(CLANG_EXECUTABLE clang REQUIRED)
set(CMAKE_C_COMPILER "${CLANG_EXECUTABLE}")
set(CMAKE_ASM_COMPILER "${CLANG_EXECUTABLE}")

# Tell Clang the target
set(CMAKE_C_COMPILER_TARGET x86_64-elf)
set(CMAKE_ASM_COMPILER_TARGET x86_64-elf)

# Use LLVM binutils
find_program(CMAKE_AR llvm-ar REQUIRED)
find_program(CMAKE_RANLIB llvm-ranlib REQUIRED)

# Prevent CMake from testing the compiler with a full program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
