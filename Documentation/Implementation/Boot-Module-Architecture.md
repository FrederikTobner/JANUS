# Boot Module Architecture Proposal

This document outlines a proposed structure for supporting multiple architectures and boot protocols in TinyOS/JANUS, with a clean separation of concerns and a scalable build system.

---

## Current State

### Boot Module

The boot module currently selects implementation files based on a simple `${ARCH}/${BOOT_PROTOCOL}` path:

```
kernel/boot/
├── CMakeLists.txt              # Selector (dispatches to arch/protocol combo)
└── x86_64/
    └── multiboot2/
        ├── CMakeLists.txt
        ├── boot.asm
        └── multiboot2.asm
```

### Linker Script (in Core)

The linker script currently lives in `kernel/core/` and is also selected by arch/protocol:

```
kernel/core/
├── CMakeLists.txt              # Selects linker script, builds kernel
├── main.c
└── x86_64/
    └── multiboot2/
        └── linker.ld           # Memory layout for x86_64 + Multiboot2
```

The linker script defines:

- Load address (`0x100000` for BIOS-based x86_64)
- Section ordering (`.multiboot` first for protocol header)
- Page alignment for sections
- Kernel boundary symbols (`kernel_start`, `kernel_end`)

### Why Linker Scripts Are Protocol-Dependent

Different boot protocols have different requirements:

| Protocol | Linker Constraints |
|----------|-------------------|
| **Multiboot2** | Header must be in first 32KB, specific section (`.multiboot`) at start |
| **Limine** | Requests in `.limine_requests` section, different load address possible |
| **UEFI** | PE/COFF format (not ELF), completely different linking strategy |

This means the linker script is inherently tied to the **arch + protocol** combination.

---

## Proposed Structure

### Design Goals

1. **Minimal restructuring** — keep current `${ARCH}/${BOOT_PROTOCOL}` dispatch pattern
2. **Clear ownership** — each component knows where its configuration lives
3. **No new logic** — only reorganize files, no new abstractions yet
4. **Extensible** — adding a new protocol or arch requires adding files, not changing existing ones

### Directory Layout

```
kernel/
├── boot/                           # Boot module (entry + protocol handling)
│   ├── CMakeLists.txt              # Dispatches to arch/protocol
│   │
│   ├── include/                    # Shared boot headers
│   │   └── boot/
│   │       └── protocol.h          # Protocol type enum, common definitions
│   │
│   └── x86_64/                     # Arch + protocol entry points
│       └── multiboot2/
│           ├── CMakeLists.txt
│           ├── boot.asm            # CPU init, jump to C (existing)
│           ├── multiboot2.asm      # Protocol header (existing)
│           └── verify.c            # Protocol verification (moved here)
│
├── core/                           # Kernel core
│   ├── CMakeLists.txt              # Selects linker script, builds kernel
│   ├── main.c
│   │
│   └── link/                       # Linker scripts (arch + protocol specific)
│       └── x86_64/
│           ├── multiboot2.ld       # Current linker.ld, renamed
│           └── limine.ld           # (future)
│
└── ...
```

### Why `verify.c` Lives in the Entry Directory

The verification code is **tightly coupled** to the entry point:

1. **Called immediately after entry** — `_start` → `verify_boot()` → `kernel_main()`
2. **Protocol-specific registers/memory** — Multiboot2 passes info in `ebx`, Limine uses memory requests
3. **May have arch-specific aspects** — 32-bit vs 64-bit pointer handling

Keeping `verify.c` alongside `boot.asm` and `multiboot2.asm` makes the entry point self-contained:

```
boot/x86_64/multiboot2/
├── multiboot2.asm      # Protocol header (what bootloader looks for)
├── boot.asm            # Entry point (what runs first)
└── verify.c            # Validation (called from boot.asm or early C)
```

Each `${ARCH}/${PROTOCOL}/` directory contains **everything** needed to boot via that combination.

### Key Changes from Current State

| Component | Current Location | Proposed Location | Rationale |
|-----------|-----------------|-------------------|-----------|
| Entry ASM | `boot/x86_64/multiboot2/` | `boot/x86_64/multiboot2/` | **No change** |
| Protocol header | `boot/x86_64/multiboot2/` | `boot/x86_64/multiboot2/` | **No change** |
| Verification | `boot/protocol/multiboot2/verify.c` | `boot/x86_64/multiboot2/verify.c` | **Moved** — colocate with entry |
| Protocol dir | `boot/protocol/` | *(removed)* | Unnecessary indirection |
| Linker script | `core/x86_64/multiboot2/linker.ld` | `core/link/x86_64/multiboot2.ld` | Flatten for clarity |
| Protocol verify | `boot/protocol/multiboot2/` | `boot/protocol/multiboot2/` | **No change** — already separated |
| Linker script | `core/x86_64/multiboot2/linker.ld` | `core/link/x86_64/multiboot2.ld` | Flatten to `link/` directory with clear naming |

### What Stays the Same

- `${JANUS_TARGET_ARCH}` and `${JANUS_BOOT_PROTOCOL}` CMake variables
- Dispatch pattern in `boot/CMakeLists.txt`
- `janus_create_kernel()` function in core

---

## Linker Script Organization

### Proposed Structure

```
kernel/core/link/
├── x86_64/
│   ├── multiboot2.ld       # BIOS boot via GRUB, Multiboot2 protocol
│   ├── limine.ld           # (future) Limine protocol
│   └── common.ld           # (optional) Shared sections, included by others
│
└── aarch64/
    └── uefi.ld             # (future) UEFI boot
```

### Selection Logic in Core CMakeLists.txt

```cmake
# kernel/core/CMakeLists.txt

# Linker script path: link/${ARCH}/${PROTOCOL}.ld
set(_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/link/${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}.ld")

if(NOT EXISTS "${_linker_script}")
    message(FATAL_ERROR
        "Linker script not found for ${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}.\n"
        "Expected: ${_linker_script}")
endif()

janus_create_kernel(
    SOURCES ${KERNEL_SOURCES}
    LIBRARIES arch boot drivers
    LINKER_SCRIPT ${_linker_script}
)
```

### Linker Script Factoring (Optional, for Future)

If multiple protocols share most of their layout, use includes:

```ld
/* kernel/core/link/x86_64/multiboot2.ld */

ENTRY(_start)

SECTIONS {
    . = 0x100000;
    
    /* Protocol-specific: Multiboot2 header first */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    /* Include common section definitions */
    INCLUDE "common.ld"
}
```

```ld
/* kernel/core/link/x86_64/common.ld */

.text ALIGN(4K) : {
    *(.text)
    *(.text.*)
}

.rodata ALIGN(4K) : {
    *(.rodata)
    *(.rodata.*)
}

/* ... etc ... */

kernel_start = 0x100000;
kernel_end = .;
```

This is **optional** — only do it when you have a second x86_64 protocol to share with.

---

## CMake Configuration

### Top-Level Variables (Unchanged)

```cmake
# Architecture selection
set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
set_property(CACHE JANUS_TARGET_ARCH PROPERTY STRINGS x86_64 aarch64)

# Boot protocol selection
set(JANUS_BOOT_PROTOCOL "multiboot2" CACHE STRING "Boot protocol")
set_property(CACHE JANUS_BOOT_PROTOCOL PROPERTY STRINGS multiboot2 limine uefi)
```

### Boot Module CMakeLists.txt

```cmake
# kernel/boot/CMakeLists.txt

# Entry point (arch + protocol specific) — contains everything for that combo
set(_entry_dir "${CMAKE_CURRENT_SOURCE_DIR}/${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}")

if(NOT EXISTS "${_entry_dir}/CMakeLists.txt")
    message(FATAL_ERROR
        "Boot entry not found for ${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}.\n"
        "Expected: ${_entry_dir}")
endif()

add_subdirectory("${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}")
```

### Entry Point CMakeLists.txt

```cmake
# kernel/boot/x86_64/multiboot2/CMakeLists.txt

enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

# All boot files for this arch+protocol combo
add_library(boot OBJECT
    multiboot2.asm          # Protocol header
    boot.asm                # Entry point, CPU init
    verify.c                # Protocol verification
)

target_include_directories(boot
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_options(boot PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${JANUS_COMMON_FLAGS}>
)
```

This is simpler than before — no cross-directory references, no conditional target linking.

---

## Adding a New Boot Protocol (Example: Limine on x86_64)

To add Limine support while keeping Multiboot2, you would add:

### 1. Entry Point Directory (Self-Contained)

```
kernel/boot/x86_64/limine/
├── CMakeLists.txt
├── limine_requests.asm         # Limine request structures
├── boot.asm                    # Entry, CPU init, jump to kernel_main
└── verify.c                    # Validate Limine responses
```

### 2. Linker Script

```
kernel/core/link/x86_64/limine.ld
```

Key differences from `multiboot2.ld`:

- Different load address (Limine can load higher-half directly)
- `.limine_requests` section instead of `.multiboot`
- Possibly different alignment requirements

### 3. No Other Changes Required

- `boot/CMakeLists.txt` dispatch logic already handles `${ARCH}/${PROTOCOL}`
- `core/CMakeLists.txt` already selects `link/${ARCH}/${PROTOCOL}.ld`
- Set `JANUS_BOOT_PROTOCOL=limine` and rebuild

---

## Adding a New Architecture (Example: aarch64)

### 1. Entry Point

```
kernel/boot/aarch64/uefi/
├── CMakeLists.txt
├── entry.asm                   # ARM64 entry, exception level setup
└── boot.c                      # UEFI boot services interaction
```

### 2. Linker Script

```
kernel/core/link/aarch64/uefi.ld
```

Note: UEFI on ARM typically produces a PE executable, not ELF. The linker script and toolchain setup differ significantly.

### 3. Arch Module

```
kernel/arch/aarch64/
├── CMakeLists.txt
└── ...                         # CPU-specific code
```

### 4. Update CMake Cache Options

```cmake
set_property(CACHE JANUS_TARGET_ARCH PROPERTY STRINGS x86_64 aarch64)
```

---

## Support Matrix

Track supported combinations:

| Architecture | Multiboot2 | Limine | Notes |
|--------------|------------|--------|-------|
| x86_64       | ✅ current | 🔲 future | Both protocols work on BIOS and UEFI via their respective bootloaders |
| aarch64      | ❌ N/A     | 🔲 future | Limine supports ARM64 |

> **Note:** Multiboot2 is loaded by GRUB (BIOS or UEFI). Limine Protocol is loaded by Limine (BIOS or UEFI). The protocol choice doesn't lock you to a firmware type.

---

## ISO Generation

### Key Concepts: Firmware vs Protocol vs Bootloader

These are often conflated but are distinct:

| Term | What It Is | Examples |
|------|-----------|----------|
| **Firmware** | What the hardware runs at power-on | BIOS, UEFI |
| **Bootloader** | Software that loads the kernel | GRUB, Limine, systemd-boot |
| **Boot Protocol** | Interface between bootloader and kernel | Multiboot2, Limine Protocol, Linux Boot Protocol |

**Critical insight:** A single bootloader can support multiple firmware types:

| Bootloader | BIOS Support | UEFI Support | Protocol |
|------------|--------------|--------------|----------|
| GRUB | ✅ | ✅ | Multiboot2 (or Linux) |
| Limine | ✅ | ✅ | Limine Protocol |

This means you can have **one kernel built for one protocol** that boots on **both BIOS and UEFI** via a hybrid ISO.

---

### Hybrid ISO Architecture (Recommended)

A single ISO can boot on both BIOS and UEFI by including:

```
iso/
├── boot/
│   ├── kernel.elf              # Your kernel (one protocol, e.g., Multiboot2)
│   └── grub/
│       ├── grub.cfg            # GRUB menu configuration
│       └── i386-pc/            # GRUB BIOS modules (added by grub-mkrescue)
│
├── EFI/
│   └── BOOT/
│       └── BOOTX64.EFI         # GRUB EFI binary (added by grub-mkrescue)
│
└── [El Torito boot record]     # For BIOS CD boot (added by grub-mkrescue)
```

**How it works:**

- BIOS → El Torito → GRUB (BIOS) → loads kernel via Multiboot2
- UEFI → EFI System Partition → GRUB (EFI) → loads kernel via Multiboot2

**Same kernel, same protocol, both firmware types.**

### CMake Hybrid ISO Generation

```cmake
# cmake/iso.cmake

function(janus_create_hybrid_iso)
    set(ISO_DIR "${CMAKE_BINARY_DIR}/iso")
    set(ISO_OUTPUT "${CMAKE_BINARY_DIR}/janus-${JANUS_TARGET_ARCH}.iso")

    # grub-mkrescue automatically creates hybrid BIOS+UEFI bootable ISOs
    add_custom_command(
        OUTPUT ${ISO_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${ISO_DIR}/boot/kernel.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in ${ISO_DIR}/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${ISO_OUTPUT} ${ISO_DIR}
        DEPENDS kernel
        COMMENT "Creating hybrid BIOS+UEFI ISO: janus-${JANUS_TARGET_ARCH}.iso"
    )

    add_custom_target(iso DEPENDS ${ISO_OUTPUT})
endfunction()
```

> **Note:** `grub-mkrescue` handles the complexity of creating a hybrid ISO that boots on BIOS (via El Torito) and UEFI (via embedded EFI partition).

---

### Supporting Multiple Protocols in One ISO (Advanced)

If you want a single ISO that can boot via **different bootloaders/protocols** (e.g., both GRUB/Multiboot2 and Limine), you have two options:

#### Option A: Multi-Protocol Kernel (Complex)

Build one kernel with **both** protocol headers:

```
kernel.elf
├── .multiboot section    # Multiboot2 header (for GRUB)
├── .limine_requests      # Limine requests (for Limine)
└── .text, .data, ...     # Kernel code
```

**Linker script for multi-protocol:**

```ld
/* kernel/core/link/x86_64/hybrid.ld */

ENTRY(_start)

SECTIONS {
    . = 0x100000;
    
    /* Multiboot2 header (must be in first 32KB) */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    /* Limine requests (can be anywhere, but keep early) */
    .limine_requests ALIGN(8) : {
        KEEP(*(.limine_requests))
    }
    
    .text ALIGN(4K) : { *(.text*) }
    /* ... rest of sections ... */
}
```

**Entry point handling:**

- Both protocols jump to `_start`
- `_start` detects which protocol booted it (check magic values in registers)
- Dispatches to appropriate protocol handler

**ISO structure:**

```
iso/
├── boot/
│   ├── kernel.elf          # Multi-protocol kernel
│   └── grub/
│       └── grub.cfg        # GRUB loads kernel via multiboot2
│
├── limine.cfg              # Limine loads same kernel via Limine protocol
├── limine-bios.sys         # Limine BIOS loader
├── limine-bios-cd.bin      # Limine BIOS CD boot
│
└── EFI/
    └── BOOT/
        ├── BOOTX64.EFI     # Could be GRUB or Limine EFI
        └── ...
```

**Pros:** One kernel, one ISO, maximum compatibility
**Cons:** Complex entry point, larger kernel, harder to debug

#### Option B: Multiple Kernels in ISO (Simpler)

Build **separate kernel binaries** for each protocol, include both in ISO:

```
iso/
├── boot/
│   ├── kernel-mb2.elf      # Kernel built with JANUS_BOOT_PROTOCOL=multiboot2
│   ├── kernel-limine.elf   # Kernel built with JANUS_BOOT_PROTOCOL=limine
│   └── grub/
│       └── grub.cfg        # Loads kernel-mb2.elf
│
├── limine.cfg              # Loads kernel-limine.elf
└── ...
```

**CMake for multi-kernel build:**

```cmake
# Build both kernel variants
foreach(PROTOCOL IN ITEMS multiboot2 limine)
    set(JANUS_BOOT_PROTOCOL ${PROTOCOL})
    # ... configure and build kernel-${PROTOCOL}.elf
endforeach()

# Create ISO with both
add_custom_target(iso
    DEPENDS kernel-multiboot2 kernel-limine
    COMMAND ...
)
```

**Pros:** Simpler kernel code, each protocol is clean
**Cons:** Larger ISO, two kernel binaries to maintain

---

### Recommended Approach for JANUS

**Start simple, extend later:**

| Phase | Firmware | Protocol | Bootloader | ISO Type |
|-------|----------|----------|------------|----------|
| **Now** | BIOS | Multiboot2 | GRUB | BIOS-only |
| **Next** | BIOS + UEFI | Multiboot2 | GRUB | Hybrid (grub-mkrescue) |
| **Future** | BIOS + UEFI | Multi-protocol | GRUB + Limine | Option A or B above |

For Phase 2 (hybrid BIOS+UEFI with current protocol):

- No kernel changes needed
- Just use `grub-mkrescue` which creates hybrid ISOs automatically
- Same Multiboot2 protocol, works on both firmware types

---

### Bootloader Choice Implications

If you're considering long-term multi-protocol support, consider your bootloader choice:

| If You Use... | Firmware Support | Protocol | Notes |
|---------------|-----------------|----------|-------|
| **GRUB** | BIOS + UEFI | Multiboot2 | Mature, complex config, hybrid ISO via `grub-mkrescue` |
| **Limine** | BIOS + UEFI | Limine Protocol | Modern, simple config, provides its own ISO tools |
| **Both** | BIOS + UEFI | Both | Multi-protocol kernel needed (Option A above) |

**Limine advantage:** If you switch to Limine, you get BIOS+UEFI support with a simpler, more modern protocol. The Limine bootloader handles both firmware types transparently.

**GRUB advantage:** Multiboot2 is well-documented, GRUB is ubiquitous, and `grub-mkrescue` makes hybrid ISOs trivial.

---

## Migration Steps

### From Current State to Proposed Structure

1. **Move verify.c into entry directory**:

   ```
   kernel/boot/protocol/multiboot2/verify.c
   → kernel/boot/x86_64/multiboot2/verify.c
   ```

2. **Update entry CMakeLists.txt** (remove relative path):

   ```cmake
   # Before
   add_library(boot OBJECT
       multiboot2.asm
       boot.asm
       ../../protocol/${JANUS_BOOT_PROTOCOL}/verify.c
   )

   # After
   add_library(boot OBJECT
       multiboot2.asm
       boot.asm
       verify.c
   )
   ```

3. **Move linker script**:

   ```
   kernel/core/x86_64/multiboot2/linker.ld
   → kernel/core/link/x86_64/multiboot2.ld
   ```

4. **Update core CMakeLists.txt** (path change only):

   ```cmake
   set(_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/link/${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}.ld")
   ```

5. **Delete empty directories**:

   ```
   kernel/boot/protocol/multiboot2/  (now empty)
   kernel/boot/protocol/             (now empty)
   kernel/core/x86_64/multiboot2/    (now empty)
   kernel/core/x86_64/               (now empty)
   ```

6. **Verify build** — should work identically

No logic changes, no new abstractions — just file reorganization.

---

## Summary

| Aspect | Current | Proposed |
|--------|---------|----------|
| Boot entry | `boot/${ARCH}/${PROTOCOL}/` | `boot/${ARCH}/${PROTOCOL}/` (unchanged) |
| Protocol verify | `boot/protocol/${PROTOCOL}/verify.c` | `boot/${ARCH}/${PROTOCOL}/verify.c` (colocated) |
| Linker script | `core/${ARCH}/${PROTOCOL}/linker.ld` | `core/link/${ARCH}/${PROTOCOL}.ld` |
| Selection logic | `${ARCH}/${PROTOCOL}` dispatch | Same pattern, clearer paths |
| Extensibility | Add folder + CMakeLists | Same — add folder + CMakeLists |

**Key insight:** Each `boot/${ARCH}/${PROTOCOL}/` directory is now **fully self-contained** — it has everything needed to boot that combination without cross-directory references. This makes adding new protocols trivial: just add a new folder with all the necessary files.
