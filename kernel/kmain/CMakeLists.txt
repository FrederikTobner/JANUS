# Kernel init module

file(GLOB INIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# Create init as an object library (linked by boot)
janus_add_subsys(kmain SOURCES ${INIT_SOURCES})

target_include_directories(kmain
    PUBLIC
        ${CMAKE_SOURCE_DIR}/kernel/include
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_SOURCE_DIR}/kernel/arch/include
        ${CMAKE_SOURCE_DIR}/kernel/subsys/drivers/include
        ${CMAKE_SOURCE_DIR}/kernel/subsys/boot/include
)

