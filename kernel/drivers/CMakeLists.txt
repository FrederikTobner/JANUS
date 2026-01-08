# Device drivers module
project(tinyos-drivers)

# Automatically find all source files
file(GLOB_RECURSE DRIVERS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE DRIVERS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(drivers STATIC)

target_include_directories(drivers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/kernel/include
)

if(DRIVERS_SOURCES)
    target_sources(drivers
        PRIVATE
            ${DRIVERS_SOURCES}
    )
endif()

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

