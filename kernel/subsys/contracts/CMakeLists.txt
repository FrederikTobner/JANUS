# Cross-subsystem contracts
#
# Each entry here is a type definition shared across a fixed set of
# subsystem boundaries.  Add to CONSUMERS only when a new subsystem
# genuinely owns or interprets the type — not merely because it receives
# it transitively through a public header chain.
#
# See cmake/kernel/Contract.cmake for the janus_add_contract() specification.

janus_add_contract(memmap
    CONSUMERS
        boot
        mm
)
