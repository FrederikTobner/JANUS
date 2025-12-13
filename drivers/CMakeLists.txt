# Device drivers module
project(tinyos-drivers)

add_library(drivers STATIC)

target_include_directories(drivers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_sources(drivers
    PRIVATE
        # Source files will be added in future phases
        # Keyboard driver
        # Disk drivers
        # Network drivers
)

target_compile_options(drivers
    PRIVATE
        -target x86_64-elf
        -nostdlib
        -ffreestanding
        -g3 -gdwarf-4
        -O0
        -Wall -Wextra
        -fno-stack-protector
        -mno-red-zone
)

target_link_libraries(drivers
    PRIVATE
        arch
        kio
)
