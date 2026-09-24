# Cross-subsystem contracts
#
# Each entry here is a type definition shared across a fixed set of subsystem boundaries. 
#
# See kernel/cmake/Contract.cmake for the janus_add_contract() specification.

janus_add_contract(memmap
    CONSUMERS
        boot
        mm
)

janus_add_contract(display
    CONSUMERS
        boot
        drivers
)
