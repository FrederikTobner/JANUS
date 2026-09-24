# Boot subsystem: protocol-agnostic core
janus_add_subsys(boot
    DEPENDENCIES
        janus_contract_memmap
        janus_contract_display
)
# Specific boot protocol implementations
foreach(protocol IN LISTS JANUS_BOOT_PROTOCOLS)
    add_subdirectory(protocol/${protocol})
endforeach()
