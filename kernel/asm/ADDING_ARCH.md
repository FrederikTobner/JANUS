# Adding A New Architecture To `kernel/asm`

This guide describes what must be added when bringing up a new architecture in the asm layer.

## Goal

After this process, the new architecture must:

- provide `arch_asm_impl_*` primitives,
- define all required capability macros,
- pass `asm/capabilities.h` validation,
- and build without touching generic callers of `asm_*` wrappers.

## 1. Add Architecture Implementation Headers

Create:

- `arch/<new_arch>/include/arch/impl/asm/cpu.h`
- `arch/<new_arch>/include/arch/impl/asm/barriers.h`
- `arch/<new_arch>/include/arch/impl/asm/tlb.h`
- `arch/<new_arch>/include/arch/impl/asm/regs.h`
- `arch/<new_arch>/include/arch/impl/asm/io.h` (only if applicable)
- `arch/<new_arch>/include/arch/impl/asm/capabilities.h`

Use `arch_asm_impl_*` naming for instruction-level primitives.

## 2. Define Required Macros In `capabilities.h`

In `arch/<new_arch>/include/arch/impl/asm/capabilities.h`, define all required macros.

Architecture selectors:

- `ASM_ARCH_X86_64`
- `ASM_ARCH_AARCH64`
- `ASM_ARCH_<NEW_ARCH>`

Capability flags:

- `ASM_CAP_LOCAL_IRQ_CONTROL`
- `ASM_CAP_IDLE_WAIT_INTERRUPT`
- `ASM_CAP_TLB_INVALIDATE_PAGE`
- `ASM_CAP_PORT_IO`
- `ASM_CAP_ARCH_SYSREG_ACCESS`

Rules:

- every macro must exist,
- every macro value must be `0` or `1`,
- exactly one `ASM_ARCH_*` selector must be `1` for a given build target.

## 3. Register The New Architecture Selector In Main Validation

Update `include/asm/capabilities.h`:

1. Add a new validation block for your selector macro:
   - presence check (`#ifndef ... #error`)
   - value check (`ASM_VALIDATE_BOOL_MACRO(...)`)

2. Update the single-architecture validation at the bottom:
   - include your new selector in the sum expression
   - keep the rule: sum must equal `1`

Example shape:

```c
#if ((ASM_ARCH_X86_64 + ASM_ARCH_AARCH64 + ASM_ARCH_NEW_ARCH) != 1)
#error "Exactly one ASM_ARCH_* macro must be set to 1"
#endif
```

## 4. Wire Build Include Path For The New Arch

Ensure the architecture exists as a valid `JANUS_TARGET_ARCH` option in your toolchain/preset flow so the existing `janus_asm` include chain can resolve:

- `kernel/asm/include`
- `kernel/asm/arch/include`
- `kernel/asm/arch/<JANUS_TARGET_ARCH>/include`

No extra asm-specific CMake wiring is needed if your target arch selection is already integrated.

## 5. Keep Wrapper/Contract/Impl Responsibilities

- Public wrappers (`include/asm/*.h`) expose stable `asm_*` APIs.
- Contract headers (`arch/include/arch/asm/*.h`) expose `arch_asm_*` operations.
- Impl headers (`arch/<new_arch>/include/arch/impl/asm/*.h`) implement `arch_asm_impl_*` primitives.

Do not export public `asm_*` functions directly from impl headers.

## 6. Validate

Build at least one preset using the new architecture and verify:

- no missing macro errors from `asm/capabilities.h`,
- no invalid-value errors (`must be 0 or 1`),
- single-architecture validation passes,
- link/build succeeds for kernel targets.
