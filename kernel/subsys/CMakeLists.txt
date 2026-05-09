include(kernel/Subsystem)

# Cross-subsystem contracts (must come before subsys so INTERFACE targets exist)
add_subdirectory(contracts)

add_subdirectory(boot)

add_subdirectory(drivers)

add_subdirectory(mm)
