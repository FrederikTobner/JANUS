# Boot subsystem: protocol-agnostic core
# Protocol implementations live in protocol/limine/ and protocol/multiboot2/

janus_add_subsys(boot)

# Protocol libraries (plain add_library, not subsystems)
add_subdirectory(protocol/limine)
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    add_subdirectory(protocol/multiboot2)
endif()
