# Kernel init module - the final assembly point

file(GLOB_RECURSE INIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# kmain links to subsystems - arch includes propagate automatically via PUBLIC
# NO_ASM: kmain is a top-level aggregation module; per the privilege model it
# must not have a direct edge to janus_asm (it only reaches asm via subsystems).
janus_add_subsys(kmain NO_ASM SOURCES ${INIT_SOURCES}
    DEPENDENCIES
        drivers
        boot
        fmt
)

# Add kmain's own include directory and generated headers
target_include_directories(kmain
    PUBLIC
        ${CMAKE_BINARY_DIR}/include  # Generated headers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/internal  
)

