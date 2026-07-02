# Kernel init module - the final assembly point
# Fault testing configuration. This is used to enable fault injection for testing purposes. 
option(JANUS_TEST_FAULTS "Enable fault injection for testing" OFF)
if(JANUS_TEST_FAULTS)
    add_compile_definitions(JANUS_TEST_FAULTS=1)
endif()
option(JANUS_TEST_FAULTS_KIND "Fault injection kind (0=stack overflow, 1=page fault)" 0)
if(JANUS_TEST_FAULTS_KIND)
    add_compile_definitions(JANUS_TEST_FAULTS_KIND=${JANUS_TEST_FAULTS_KIND})
endif()


file(GLOB_RECURSE INIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# kmain links to subsystems - arch includes propagate automatically via PUBLIC
janus_add_subsys(kmain SOURCES ${INIT_SOURCES}
    DEPENDENCIES
        drivers
        boot
        fmt
        interrupts
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

