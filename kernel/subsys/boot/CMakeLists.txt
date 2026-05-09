# Boot subsystem: protocol-agnostic core
# Protocol implementations live in protocol/limine/ and protocol/multiboot2/

janus_add_subsys(boot
    DEPENDENCIES
        display
        janus_contract_memmap
)

# Protocol libraries (plain add_library, not subsystems)
add_subdirectory(protocol/limine)
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    # Multiboot2 is only supported on x86_64, so only add that protocol if we're building for that architecture.
    add_subdirectory(protocol/multiboot2)
endif()
