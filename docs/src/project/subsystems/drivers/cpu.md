# CPU Driver

The CPU driver currently provides a single function: `drivers_cpu_halt_forever()`, which halts the processor in an infinite loop. As the kernel grows, this module will expand to cover interrupt control and processor state management.

## Source Files

| File | Description |
|------|-------------|
| `include/drivers/cpu.h` | Public API — `drivers_cpu_halt_forever()` |
| `arch/include/arch/drivers/cpu.h` | Architecture contract — each arch must implement `arch_cpu_halt_forever()` |
| `arch/x86_64/include/arch/impl/drivers/cpu.h` | x86_64 implementation — inline `cli` + `hlt` loop |
| `arch/aarch64/include/arch/impl/drivers/cpu.h` | aarch64 implementation — inline `wfi` loop |

## Design

The halt function is implemented as an inline function in each architecture's `impl` header, because it consists of a single tight loop around a privileged instruction. This avoids the overhead of a function call for what is essentially the kernel's final instruction.
