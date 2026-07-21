# interrupts — Interrupt & Exception Handling Subsystem

Installs CPU interrupt/exception handling so that faults become readable panics over `kio` instead of silent triple faults and resets.

## Verification

Build with `-DJANUS_TEST_FAULTS=ON` to trigger a deliberate fault from `kmain` after the IDT is active, and inspect the panic over serial in QEMU.
Select thefault with `-DJANUS_FAULT_TEST_KIND=0` (page fault, default) or `=1` (double fault via stack overflow).
