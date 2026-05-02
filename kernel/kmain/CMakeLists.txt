# Kernel init module - the final assembly point

file(GLOB_RECURSE INIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# kmain links to subsystems - arch includes propagate automatically via PUBLIC
janus_add_subsys(kmain SOURCES ${INIT_SOURCES}
    DEPENDENCIES
        drivers
        boot
        fmt
        kio
        mm
)

# Add kmain's own include directory and generated headers
target_include_directories(kmain
    PUBLIC
        ${CMAKE_BINARY_DIR}/include  # Generated headers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/internal  
)

