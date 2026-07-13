# interrupts — Interrupt & Exception Handling Subsystem

Installs CPU interrupt/exception handling so that faults become readable panics over `kio` instead of silent triple faults and resets.

> **x86_64 only.** On aarch64 this library is not implemented. All functions are no-ops under aarch64

## x86_64 details

- **IDT**: 256 interrupt gates; exception vectors use interrupt gates (clear the interrupt-enable flag on entry). All vectors are populated, so an unexpected vector panics rather than triple-faulting.
- **GDT/TSS/IST**: a kernel-owned GDT with a TSS provides an Interrupt Stack Table; the `#DF` (Double Fault) gate runs on a dedicated stack so it survives a corrupt/exhausted main stack.
- **Reporting**: handlers emit the vector + mnemonic, error code, `RIP`,all the fugeneral-purpose register set, and — for `#PF` — `CR2`, then `kpanic`.

## Verification

Build with `-DJANUS_TEST_FAULTS=ON` to trigger a deliberate fault from `kmain` after the IDT is active, and inspect the panic over serial in QEMU.
Select thefault with `-DJANUS_FAULT_TEST_KIND=0` (page fault, default) or `=1` (double fault via stack overflow).
