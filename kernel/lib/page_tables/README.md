# page\_tables — AArch64 MMU Library

Stateless library for creating and managing 4-level (L0–L3) AArch64 page tables.

> **AArch64 only.** On x86_64 this library is a no-op INTERFACE target.

## Public API

```c
#include <page_tables/mmu.h>

// Initialise with HHDM offset and kernel physical/virtual base addresses.
void        mmu_init(u64 hhdm_offset, phys_addr_t kernel_phys, virt_addr_t kernel_virt);

// Map a physical MMIO region into the kernel virtual address space.
// Returns the virtual address, or 0 on failure.
virt_addr_t mmu_map_mmio(phys_addr_t phys_addr, u64 size);
```

## Internals

- Uses a static pool of 8 pre-allocated page table pages.
- Reads the TTBR1\_EL1 root via `asm/regs.h` (`asm_read_ttbr1_el1`).
- Invalidates TLB entries per mapped page via `asm/tlb.h` (`asm_tlbi_vale1is`).
- Issues `DSB SY` + `ISB` barriers after the flush via `asm/barriers.h`.

## Dependencies

`janus_asm` (for `asm/barriers.h`, `asm/tlb.h`, `asm/regs.h`) — listed
explicitly in `DEPENDENCIES` in `lib/page_tables/CMakeLists.txt`.
