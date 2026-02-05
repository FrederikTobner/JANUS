# Multi-Architecture Support Analysis for JANUS

> **Analysis Date:** February 2, 2026  
> **Target Architecture:** ARM64 (AArch64) — Raspberry Pi 3 Model B  
> **Current Architecture:** x86_64

---

## Executive Summary

JANUS is **well-positioned** for multi-architecture support. The project demonstrates thoughtful architecture from the start, with a well-defined three-tier architecture abstraction layer and modular CMake build system. However, some concrete work remains to achieve a clean ARM64 port.

| Category | Status | Rating |
|----------|--------|--------|
| Architecture Abstraction | Strong foundation | ⭐⭐⭐⭐☆ |
| Build System | Ready with minor extensions | ⭐⭐⭐⭐☆ |
| Boot Protocol | Limine is portable; Multiboot2 is x86-only | ⭐⭐⭐⭐☆ |
| Kernel Code Portability | Mostly portable, some work needed | ⭐⭐⭐☆☆ |
| Driver Architecture | Excellent abstraction pattern | ⭐⭐⭐⭐⭐ |

---

## 1. Architecture Layer Analysis

### 1.1 Three-Tier Model — Excellent Design ✅

Your documented three-tier architecture pattern is industry-standard and will scale well:

```
Tier 1 (Public API)     →  include/drivers/tty.h
Tier 2 (Arch Contract)  →  arch/include/arch/drivers/tty.h  
Tier 3 (Impl Details)   →  arch/x86_64/include/arch/impl/drivers/*.h
```

**Strengths:**

- Clean separation between portable and arch-specific code
- Contract headers (`arch/drivers/*.h`) define the interface once
- Implementation is hidden from consumers
- CMake auto-detects and fails fast if arch implementation is missing

**Example from your codebase:**

```c
// tty.c (portable) — calls arch contract functions
#include <arch/drivers/tty.h>
arch_tty_write_cell(x, y, ch, fg, bg);  // Defined per-arch

// arch/x86_64/tty.c — implements the contract
error_t arch_tty_init(tty_display_config_t const * config) { ... }
```

**For ARM64:** Simply create `arch/aarch64/` directories with implementations. No changes to generic code needed.

### 1.2 Current Arch Code Distribution

| Subsystem | Has `arch/` | x86_64 Impl | ARM64 Ready |
|-----------|-------------|-------------|-------------|
| `drivers/` | ✅ | ✅ serial.c, tty.c, framebuffer.c | ⬜ Needs impl |
| `boot/` | ❌ | Protocol code in `internal/` | ⬜ Need to refactor |
| `mm/` | ❌ | Placeholder | ⬜ Will need arch layer |
| `_start/` | ✅ | ✅ limine/, multiboot2/ | ⬜ Needs aarch64/ |

---

## 2. Build System Analysis

### 2.1 CMake Platform Layer — Good Foundation ✅

[JanusPlatform.cmake](cmake/JanusPlatform.cmake) already has architecture awareness:

```cmake
set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture")
```

**Strengths:**

- Architecture is a configurable variable
- Compiler flags are conditionally set per-arch
- QEMU target selection already handles `aarch64`

**Required Extensions for Cross-Compilation:**

The architecture should remain configurable at CMake configure time, defaulting to x86_64:

```bash
# Default build (x86_64)
cmake -B build -G Ninja

# Cross-compile for ARM64
cmake -B build-arm64 -G Ninja -DJANUS_TARGET_ARCH=aarch64
```

Extend `JanusPlatform.cmake` for AArch64 support:

```cmake
# Architecture selection with x86_64 default
set(JANUS_TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86_64, aarch64)")
set_property(CACHE JANUS_TARGET_ARCH PROPERTY STRINGS "x86_64" "aarch64")

# Validate architecture selection
if(NOT JANUS_TARGET_ARCH MATCHES "^(x86_64|aarch64)$")
    message(FATAL_ERROR "Unsupported architecture: ${JANUS_TARGET_ARCH}")
endif()

# Cross-compilation detection
if(NOT CMAKE_CROSSCOMPILING)
    # Native build - check if target matches host
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64" AND JANUS_TARGET_ARCH STREQUAL "x86_64")
        set(JANUS_CROSS_COMPILING TRUE)
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64" AND JANUS_TARGET_ARCH STREQUAL "aarch64")
        set(JANUS_CROSS_COMPILING TRUE)
    endif()
endif()

if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    if(JANUS_COMPILER_CLANG)
        set(JANUS_COMMON_FLAGS
            -target aarch64-elf
            -mcpu=cortex-a53        # RPi 3B CPU
            -nostdlib
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -Wall -Wextra -Werror
            -Wconversion -Wcast-qual -Wpointer-arith
        )
    else()
        # GCC cross-compiler
        set(JANUS_COMMON_FLAGS
            -mcpu=cortex-a53
            -nostdlib
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -Wall -Wextra -Werror
        )
    endif()
endif()
```

### 2.2 Subsystem Auto-Detection — Excellent ✅

[JanusSubsys.cmake](cmake/JanusSubsys.cmake) automatically detects arch folders:

```cmake
set(ARCH_IMPL_DIR "${ARCH_DIR}/${JANUS_TARGET_ARCH}")
if(NOT EXISTS "${ARCH_IMPL_DIR}")
    message(FATAL_ERROR 
        "Subsystem '${NAME}' has arch/ but missing implementation for ${JANUS_TARGET_ARCH}")
endif()
```

This means adding ARM64 is as simple as creating the directory — the build system will find it.

### 2.3 Entry Point Dispatch — Correct Pattern ✅

[kernel/_start/CMakeLists.txt](kernel/_start/CMakeLists.txt) dispatches by architecture:

```cmake
add_subdirectory("${JANUS_TARGET_ARCH}")  # Automatically picks x86_64 or aarch64
```

**For ARM64:** Create `kernel/_start/aarch64/` with Limine entry point.

---

## 3. Boot Protocol Analysis

### 3.1 Limine — Portable to ARM64 ✅

**Limine supports both x86_64 and AArch64**, making it your best path forward for the Raspberry Pi 3B.

**Current Limine setup:**

- Higher-half kernel design (virtual address `0xFFFFFFFF80000000`)
- HHDM (Higher Half Direct Map) offset handling
- Framebuffer request structure

**ARM64 considerations:**

- Limine on ARM uses device tree instead of ACPI
- Memory map format is the same
- Framebuffer protocol is identical

**Your linker script comment confirms awareness:**

```ld
/* This linker script is also portable to AArch64 with Limine. */
```

### 3.2 Multiboot2 — x86-Only (Expected) ⚠️

Multiboot2 is inherently x86-specific. Your build system already handles this:

```cmake
# In JanusTargets.cmake
add_custom_target(iso-grub
    COMMAND echo "ERROR: GRUB ISO only available for x86_64"
```

**Recommendation:** Your consideration to potentially drop Multiboot2 support makes sense if ARM becomes your primary target. However, keeping it for x86_64 development/testing is low-cost since the abstraction is already in place.

### 3.3 VGA Text Mode — x86-Only (Expected) ⚠️

VGA text mode (`0xB8000`) is x86-specific hardware. Your code already handles this gracefully:

```c
// arch/x86_64/tty.c
typedef enum {
    DISPLAY_MODE_NONE,
    DISPLAY_MODE_VGA,         // x86 only
    DISPLAY_MODE_FRAMEBUFFER, // Universal
} display_mode_t;
```

**For ARM64:** Only framebuffer mode will be available, which is already implemented.

---

## 4. Code Portability Analysis

### 4.1 Type System — Portable ✅

[uapi/types.h](kernel/include/uapi/types.h) uses compiler builtins:

```c
typedef __UINT64_TYPE__ __u64;  // Compiler knows the right size
typedef __INT32_TYPE__ __s32;
```

This is fully portable across architectures.

### 4.2 Assembly Code — Needs ARM64 Versions ⬜

| File | Purpose | ARM64 Action |
|------|---------|--------------|
| `_start/x86_64/common/gdt64.asm` | GDT setup | Not needed on ARM |
| `_start/x86_64/limine/entry.asm` | Entry point | Needs `aarch64/limine/entry.S` |
| `_start/x86_64/multiboot2/*.asm` | MB2 header/entry | Not applicable |

**Note:** ARM64 assembly typically uses `.S` extension (GAS syntax) rather than `.asm` (NASM).

### 4.3 I/O Operations — Needs Abstraction ⬜

The x86-specific `outb`/`inb` port I/O in [arch/x86_64/serial.c](kernel/subsys/drivers/arch/x86_64/serial.c):

```c
#include <arch/impl/drivers/io.h>
outb(COM1_INT_EN, 0x00);
```

**ARM64 equivalent:** Memory-mapped I/O (MMIO). The Raspberry Pi 3B uses MMIO for UART:

```c
// Future: arch/aarch64/serial.c
#define UART0_BASE  0x3F201000  // BCM2837 PL011 UART
volatile u32 *uart = (volatile u32 *)UART0_BASE;
uart[DATA_REG] = byte;
```

Your arch layer already supports this — just implement `arch_serial_*` for AArch64.

### 4.4 CPU Operations — Needs ARM64 Implementations ⬜

`drivers_cpu_halt_forever()` uses x86 `hlt` instruction. ARM64 equivalent:

```c
// arch/aarch64/cpu.c
void arch_cpu_halt(void) {
    __asm__ volatile("wfi");  // Wait For Interrupt
}
```

---

## 5. Raspberry Pi 3B Specific Considerations

### 5.1 Hardware Overview

| Component | Detail |
|-----------|--------|
| CPU | Broadcom BCM2837, Cortex-A53 (ARMv8-A, 64-bit) |
| RAM | 1GB LPDDR2 |
| Boot | GPU loads kernel from SD card |
| UART | PL011 at `0x3F201000` (useful for early debug) |
| Framebuffer | Mailbox interface to VideoCore GPU |

### 5.2 Boot Process Differences

**x86_64 with Limine:**

```
BIOS/UEFI → Limine → kernel_main()
```

**RPi 3B with Limine (planned):**

```
GPU ROM → config.txt → Limine → kernel_main()
```

Limine handles this difference — your kernel entry point remains the same.

### 5.3 Required New Files for ARM64

```
kernel/
├── _start/
│   └── aarch64/
│       ├── CMakeLists.txt
│       ├── link-limine.ld       # AArch64 linker script
│       └── limine/
│           ├── entry.S          # ARM64 assembly entry
│           └── requests.c       # Same as x86 (C code)
└── subsys/
    └── drivers/
        └── arch/
            └── aarch64/
                ├── include/arch/impl/drivers/
                │   └── mmio.h   # Memory-mapped I/O helpers
                ├── cpu.c        # WFI halt, etc.
                ├── serial.c     # PL011 UART driver
                └── tty.c        # Framebuffer only (no VGA)
```

---

## 6. Gap Analysis & Action Items

### 6.1 High Priority (Required for ARM64 Boot)

| Task | Effort | Files Affected |
|------|--------|----------------|
| Add `aarch64` to `JanusPlatform.cmake` | Low | 1 file |
| Create `_start/aarch64/` entry point | Medium | ~4 new files |
| Implement `arch/aarch64/cpu.c` | Low | 1 new file |
| Implement `arch/aarch64/serial.c` (PL011) | Medium | 1 new file |
| Implement `arch/aarch64/tty.c` (FB only) | Low | 1 new file |

### 6.2 Medium Priority (Polish)

| Task | Effort | Notes |
|------|--------|-------|
| Add MMIO header for ARM | Low | `arch/impl/drivers/mmio.h` |
| Device tree parsing | Medium | Limine provides this |
| GPIO driver for RPi | Medium | For LED blink demo |

### 6.3 Low Priority (Nice to Have)

| Task | Notes |
|------|-------|
| Drop Multiboot2 support | Optional, reduces maintenance |
| Drop VGA text mode | Can keep for x86 nostalgia |
| Implement interrupts (GIC) | Needed for real OS, not for boot |

---

## 7. Recommendations

### 7.1 Keep Your Current Architecture ✅

Your three-tier arch layer and subsystem isolation are exactly what's needed. Don't change the fundamental design.

### 7.2 Use Limine as Primary Bootloader ✅

You're already treating Limine as primary. This is correct for multi-arch:

- Same request protocol on both architectures
- Same framebuffer interface
- Same memory map format

### 7.3 Consider Conditional Protocol Support

Update `JanusPlatform.cmake` to automatically filter protocols by architecture:

```cmake
if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    # Multiboot2 is not available on ARM
    list(REMOVE_ITEM JANUS_BOOT_PROTOCOLS "multiboot2")
endif()
```

### 7.4 NASM vs GAS for Assembly

Your x86 code uses NASM. For ARM64, you'll need GAS (GNU Assembler) since NASM doesn't support ARM. Update CMake:

```cmake
if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    enable_language(ASM)  # GAS, not ASM_NASM
endif()
```

### 7.5 Unified Build Targets

The existing `run`, `debug`, `iso` targets should work for both architectures. Extend `JanusTargets.cmake` to dispatch based on `JANUS_TARGET_ARCH`:

```cmake
# QEMU executable selection
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    set(QEMU_EXECUTABLE qemu-system-x86_64)
    set(QEMU_MACHINE_FLAGS -enable-kvm -cpu host)  # KVM if native
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    set(QEMU_EXECUTABLE qemu-system-aarch64)
    set(QEMU_MACHINE_FLAGS -M virt -cpu cortex-a72)  # Emulation
endif()

# Unified 'run' target dispatches to correct QEMU
add_custom_target(run
    COMMAND ${QEMU_EXECUTABLE} ${QEMU_MACHINE_FLAGS}
        -m 512M
        -serial stdio
        ${QEMU_KERNEL_ARG}  # -kernel or -cdrom depending on boot method
    DEPENDS kernel-limine.elf
    COMMENT "Running JANUS on QEMU (${JANUS_TARGET_ARCH})"
)
```

This allows the same workflow regardless of target:

```bash
# x86_64 (default)
ninja run          # Uses qemu-system-x86_64 with KVM

# ARM64 (cross-compiled build directory)
cd build-arm64
ninja run          # Uses qemu-system-aarch64 in emulation
```

### 7.6 Testing Strategy

1. **QEMU first:** `ninja run` handles the correct emulator automatically
2. **Serial console:** Primary debug output on both architectures
3. **Cross-compile:** Use `clang --target=aarch64-elf` (already in JANUS_COMMON_FLAGS)

---

## 8. Preparatory Work Before ARM64 Implementation

These tasks can be done **now**, while still working on x86_64, to make the ARM64 port smoother:

### 8.1 Refactor Arch-Specific Code Out of Generic Files

**Priority: High**

Audit and move any x86-specific code that leaked into generic files:

| Check | Location | Action |
|-------|----------|--------|
| Port I/O assumptions | Any `.c` outside `arch/` | Ensure no `outb`/`inb` calls |
| VGA buffer address | Generic code | Should only be in `arch/x86_64/` |
| Inline assembly | Generic headers | Move to arch contract headers |

### 8.2 Create Arch Contract Stubs

Create empty `aarch64/` directories with stub implementations that fail gracefully. This validates the build system without writing real ARM code:

```c
// kernel/subsys/drivers/arch/aarch64/cpu.c
#include <arch/drivers/cpu.h>

void arch_cpu_halt(void) {
    while (1) { }  // Stub: infinite loop
}
```

### 8.3 Abstract the Assembler Language in CMake

Prepare `_start/CMakeLists.txt` to handle both NASM (x86) and GAS (ARM):

```cmake
if(JANUS_TARGET_ARCH STREQUAL "x86_64")
    enable_language(ASM_NASM)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
elseif(JANUS_TARGET_ARCH STREQUAL "aarch64")
    enable_language(ASM)
    # GAS uses .S files, auto-detected
endif()
```

### 8.4 Validate Limine Request Portability

The Limine request structures in `_start/x86_64/limine/requests.asm` can be rewritten in C for portability:

```c
// _start/common/limine_requests.c (shared by both arches)
#include <limine.h>

__attribute__((section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
```

This eliminates arch-specific assembly for Limine protocol handling.

### 8.5 Document Arch-Specific Behaviors

Create a reference table of architecture differences to guide implementation:

| Feature | x86_64 | AArch64 |
|---------|--------|--------|
| Halt instruction | `hlt` | `wfi` |
| I/O model | Port I/O (`in`/`out`) | MMIO only |
| Page size | 4KB (default) | 4KB, 16KB, or 64KB |
| Interrupt controller | APIC/PIC | GIC |
| Serial | 16550 UART (COM1) | PL011 UART |
| Boot protocol | Limine, Multiboot2 | Limine only |

---

## 9. Conclusion

JANUS is architecturally prepared for ARM64 support. The key strengths are:

1. **Clean arch abstraction** — Add `aarch64/` folders, not new abstractions
2. **Limine-first approach** — Already chose the portable bootloader
3. **Subsystem isolation** — Each subsystem manages its own arch code
4. **CMake auto-detection** — Build system will find new arch implementations
5. **Unified targets** — Same `ninja run` workflow for both architectures

The main work is implementing the ARM64-specific drivers and entry point — the framework is ready.

**Estimated effort:**

| Phase | Time |
|-------|------|
| Preparatory refactoring (Section 8) | 1-2 days |
| ARM64 boot on QEMU | 2-4 days |
| Real Raspberry Pi 3B hardware | +1 day |

---

## References

- [Limine Boot Protocol Specification](https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md)
- [BCM2837 ARM Peripherals Manual](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2837/README.md)
- [OSDev Wiki: Raspberry Pi Bare Bones](https://wiki.osdev.org/Raspberry_Pi_Bare_Bones)
- [ARM Cortex-A53 Technical Reference Manual](https://developer.arm.com/documentation/ddi0500/latest/)
