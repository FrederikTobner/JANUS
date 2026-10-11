# aarch64 interrupts

AArch64 EL1 CPU-exception handling: turns synchronous faults into a readable diagnostic panic over `kio`.

## Components

- **`init.c`** — `arch_interrupts_init()`; delegates to `exceptions_install()`.
- **`exceptions.c`** — `exceptions_install()`; points `VBAR_EL1` at the vector table (via the capability-guarded `asm_load_interrupt_vectors` wrapper: `MSR VBAR_EL1` + `ISB`).
- **`vector_table.S`** — the 2 KiB-aligned, 16-entry (0x80 bytes each) EL1 vector table plus the shared `exc_common` save/dispatch path. Hardware saves no GPRs on aarch64, so the entry path saves `x0`–`x30` + `sp` and snapshots `ELR_EL1`/`SPSR_EL1`/`ESR_EL1`/`FAR_EL1` into an `interrupt_frame_t`.
- **`handlers.c`** — `interrupts_dispatch()`; decodes the exception class (EC) from `ESR_EL1`, prints the source label, syndrome, faulting address (for abort/alignment classes), and the full register set, then `kpanic`s.
- **`include/arch/impl/interrupts/frame.h`** — `interrupt_frame_t` and the byte offsets/size shared with `vector_table.S` (kept in lockstep by `STATIC_ASSERT`s).
- **`include/arch/impl/interrupts/vectors.h`** — EC constants, the 16-entry source-index enum, and the decoding declarations.

## Design notes

- The kernel runs at EL1t (SPSel = 0, SP_EL0 active) everywhere outside exception entry, so genuine faults arrive through the "Current EL SP0 / Synchronous" entry (source index 0). Every other entry is populated defensively and only ever panics as "unexpected". `exceptions_install()` asserts this SPSel invariant at boot before installing the vector table.
- SP_EL1 is intentionally never used as a normal stack: `exceptions.c` points it at a dedicated `.bss` stack before the vectors go live, so it survives a corrupted SP_EL0 the same way the x86 IST `#DF` stack does. Entry to EL1 (from `_start_limine`) normalizes SPSel to 0 before calling `kernel_main`, whether the CPU arrived via the EL2 drop or directly at EL1.
- The page-fault self-test (`JANUS_TEST_FAULTS`, kind 0) writes to `0x100000000` → a **Data Abort** (EC `0x25`) with `FAR_EL1 == 0x100000000`.

## Not yet implemented (future work)

- Asynchronous interrupt (IRQ/FIQ) servicing — `DAIF` stays masked; those entries panic if reached.
- Exception recovery/return (`ERET`) — currently every entry is fatal.
