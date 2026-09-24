list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Subsystem)

# Cross subsystem contracts
add_subdirectory(contracts)

add_subdirectory(boot)
add_subdirectory(drivers)
add_subdirectory(interrupts)
add_subdirectory(mm)
