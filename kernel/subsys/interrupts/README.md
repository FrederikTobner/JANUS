# interrupts — Interrupt & Exception Handling Subsystem

Installs CPU interrupt/exception handling so that faults become readable panics
over `kio` instead of silent triple faults and resets.

## Public API

- `include/interrupts/interrupts.h`
- `error_t interrupts_init(void)` — install and activate exception handling on
  the current CPU. Must be called once, after console init and before any
  fault-prone subsystem (e.g. the PMM).

The public surface is architecture-agnostic: no x86 concept (IDT, CR2, gate
type) appears above the arch contract.

## Architecture layout

- Tier 1 (public): `include/interrupts/interrupts.h`
- Tier 2 (arch contract): `arch/include/arch/interrupts/interrupts.h`
  (`arch_interrupts_init`)
- Tier 3 (x86_64 impl): `arch/x86_64/`
  - `init.c` — `arch_interrupts_init`: GDT → IDT install order
  - `gdt.c` — kernel GDT + TSS + dedicated IST stack for `#DF`
  - `idt.c` — builds all 256 IDT gates and loads via `LIDT`
  - `handlers.c` — `interrupts_dispatch` + diagnostic register/CR2 dump
  - `isr.asm` — 256 entry stubs + `isr_stub_table`
  - `include/arch/impl/interrupts/` — `frame.h`, `idt.h`, `gdt.h`, `vectors.h`
  - `internal/arch/internal/interrupts/setup.h` — subsystem-private decls
- aarch64: `arch/aarch64/exceptions.c` — stub (VBAR_EL1 vector table is TODO)

## x86_64 details

- **IDT**: 256 interrupt gates; exception vectors use interrupt gates (clear the
  interrupt-enable flag on entry). All vectors are populated, so an unexpected
  vector panics rather than triple-faulting.
- **GDT/TSS/IST**: a kernel-owned GDT with a TSS provides an Interrupt Stack
  Table; the `#DF` (Double Fault) gate runs on a dedicated stack so it survives a
  corrupt/exhausted main stack.
- **Reporting**: handlers emit the vector + mnemonic, error code, `RIP`, the full
  general-purpose register set, and — for `#PF` — `CR2`, then `kpanic`.

## Dependencies

- `kio` (core) for `kprintf`/`kpanic`
- `janus_asm` for `LIDT`/`LGDT`/`LTR` and `CR2` access

## Verification

Build with `-DJANUS_TEST_FAULTS=ON` to trigger a deliberate fault from `kmain`
after the IDT is active, and inspect the panic over serial in QEMU. Select the
fault with `-DJANUS_FAULT_TEST_KIND=0` (page fault, default) or `=1` (double
fault via stack overflow).

## Not in scope (deferred)

- 8259 PIC / I/O APIC configuration and hardware IRQ routing
- Interrupt-driven drivers (timer, keyboard)
- aarch64 exception vector implementation
