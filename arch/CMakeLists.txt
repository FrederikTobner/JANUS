# Architecture-specific code
project(tinyos-arch)

# x86_64 architecture
add_subdirectory(x86_64)

# Architecture interface library
add_library(arch INTERFACE)

target_include_directories(arch
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link x86_64 implementation (will add more architectures later)
target_link_libraries(arch INTERFACE arch_x86_64)
