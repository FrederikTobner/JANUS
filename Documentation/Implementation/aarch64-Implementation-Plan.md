# aarch64 Implementation Plan

> Feature design plan for adding ARM64 support to JANUS kernel.
>
> **Scope:** Initial implementation targets QEMU `-M virt` only. Real hardware
> (Raspberry Pi 3B) support deferred until Device Tree (DTB) parsing is added.

---

## Todo List

```yaml
todos:
  - id: 1
    name: "Build System Foundation"
    description: >
      Add aarch64 toolchain detection, cross-compilation support via
      JANUS_TARGET_ARCH, and assembler configuration (GAS for .S files).
      Create platform detection in JanusPlatform.cmake.
    priority: critical
    dependencies: []
    estimated_effort: medium
    deliverables:
      - "Modified cmake/JanusPlatform.cmake with AARCH64 detection"
      - "Cross-compilation flags for aarch64-linux-gnu-gcc"
      - "ASM language enabled for GAS alongside NASM"

  - id: 2
    name: "aarch64 Entry Point"
    description: >
      Create kernel/_start/aarch64/ directory structure with Limine entry
      point in GAS assembly. Set up stack, EL2→EL1 transition, MMU stub,
      and jump to kernel_main with proper calling convention.
    priority: critical
    dependencies: [1]
    estimated_effort: high
    deliverables:
      - "kernel/_start/aarch64/limine/entry.S"
      - "kernel/_start/aarch64/CMakeLists.txt"
      - "Linker script kernel/_start/aarch64/limine/linker.ld"

  - id: 3
    name: "Shared Limine Requests Verification"
    description: >
      Verify that kernel/_start/common/limine_requests.c compiles and
      links correctly for aarch64. Ensure section attributes work with
      aarch64-linux-gnu-gcc.
    priority: critical
    dependencies: [1, 2]
    estimated_effort: low
    deliverables:
      - "Build verification for shared limine_requests.c"
      - "Potential attribute adjustments if needed"

  - id: 4
    name: "aarch64 CPU Driver"
    description: >
      Create arch/impl/drivers/cpu.h for aarch64 with inline implementations
      of arch_cpu_halt(), arch_cpu_halt_forever(), arch_cpu_disable_interrupts(),
      arch_cpu_enable_interrupts(). Uses WFI, MSR DAIF instructions.
      Fully inlined for zero overhead (matches x86_64 pattern).
    priority: critical
    dependencies: [1]
    estimated_effort: low
    deliverables:
      - "kernel/subsys/drivers/arch/aarch64/include/arch/impl/drivers/cpu.h"

  - id: 5
    name: "aarch64 Serial Driver (PL011)"
    description: >
      Implement arch_serial_* for PL011 UART at QEMU virt address
      (0x09000000). Support initialization, write, read, and status
      checks. This enables early debug output. Hardcode QEMU virt
      address for now; real hardware addresses deferred to DTB phase.
      Architecture contract already exists at arch/include/arch/drivers/serial.h.
    priority: critical
    dependencies: [1]
    estimated_effort: medium
    deliverables:
      - "kernel/subsys/drivers/arch/aarch64/serial.c"
      - "kernel/subsys/drivers/arch/aarch64/include/arch/impl/drivers/mmio.h"
      - "PL011 register definitions (hardcoded 0x09000000)"

  - id: 6
    name: "aarch64 TTY Driver (Framebuffer)"
    description: >
      Implement arch_tty_* for aarch64 using framebuffer text rendering.
      Move existing x86_64 framebuffer rendering code to
      arch/shared/drivers/ so both architectures can use it.
      Each arch CMakeLists.txt includes shared sources explicitly.
      Architecture contract already exists at arch/include/arch/drivers/tty.h.
    priority: high
    dependencies: [5]
    estimated_effort: medium
    deliverables:
      - "kernel/subsys/drivers/arch/aarch64/tty.c"
      - "kernel/subsys/drivers/arch/aarch64/CMakeLists.txt"
      - "kernel/subsys/drivers/arch/shared/drivers/framebuffer.c (moved)"
      - "kernel/subsys/drivers/arch/shared/drivers/include/arch/shared/drivers/framebuffer.h"
      - "kernel/subsys/drivers/arch/shared/drivers/include/arch/shared/drivers/terminus.h"

  - id: 7
    name: "Verify x86_64 Still Works"
    description: >
      After moving framebuffer code to shared location, verify x86_64
      build and boot still work correctly. Update x86_64 tty.c to use
      shared framebuffer code.
    priority: high
    dependencies: [6]
    estimated_effort: low
    deliverables:
      - "Updated kernel/subsys/drivers/arch/x86_64/tty.c includes"
      - "Build and boot verification"

  - id: 8
    name: "QEMU Launch Target"
    description: >
      Add ninja run target for aarch64 that launches qemu-system-aarch64
      with -M virt machine type. This is the only supported target until
      DTB parsing is implemented.
    priority: high
    dependencies: [2]
    estimated_effort: low
    deliverables:
      - "CMake custom target for aarch64 QEMU (-M virt)"
      - "Documentation for required QEMU packages"

  - id: 9
    name: "Unified Build Integration"
    description: >
      Finalize the dual-arch build system so that ninja kernel-limine.elf
      builds for the configured JANUS_TARGET_ARCH. Ensure ninja run
      launches the correct QEMU for the target.
    priority: high
    dependencies: [1, 2, 8]
    estimated_effort: medium
    deliverables:
      - "Unified ninja run target"
      - "Build documentation updates"

  - id: 10
    name: "Documentation and Testing"
    description: >
      Update all documentation to reflect aarch64 support. Add setup
      instructions for cross-compilation toolchain. Verify boot on
      QEMU-aarch64 (-M virt). Real hardware testing deferred to DTB phase.
    priority: medium
    dependencies: [9]
    estimated_effort: low
    deliverables:
      - "Updated Documentation/Setup.md"
      - "QEMU testing verification"

  - id: 11
    name: "Device Tree (DTB) Parsing"
    description: >
      Implement Device Tree Blob parsing to support multiple boards with
      a single kernel binary. Limine provides DTB pointer; parse it to
      discover UART base address, memory layout, and other peripherals.
      This enables real Raspberry Pi 3B/4 hardware support.
    priority: low
    phase: future
    dependencies: [10]
    estimated_effort: high
    deliverables:
      - "DTB parser (libfdt or minimal custom implementation)"
      - "Runtime peripheral discovery"
      - "Real Raspberry Pi 3B boot verification"
      - "Real Raspberry Pi 4 boot verification (bonus)"
```

---

## Architecture Overview

### Current State

```
┌─────────────────────────────────────────────────────────────────────┐
│                        JANUS Kernel (x86_64 only)                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   kernel_main() ◄─── entry.asm ◄─── Limine Bootloader              │
│        │                                                            │
│        ▼                                                            │
│   ┌─────────────┐                                                   │
│   │  Subsystems │ ─── drivers, mm, etc.                            │
│   └─────────────┘                                                   │
│        │                                                            │
│        ▼                                                            │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │ arch/x86_64/   (serial.c, framebuffer.c, tty.c)             │  │
│   │                 Uses outb/inb for port I/O                   │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Target State

```
┌─────────────────────────────────────────────────────────────────────┐
│                     JANUS Kernel (multi-arch)                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   kernel_main() ◄─── entry.asm/.S ◄─── Limine Bootloader           │
│        │              (arch-specific)     (portable)                │
│        │                                                            │
│        ▼                                                            │
│   ┌─────────────┐                                                   │
│   │  Subsystems │ ─── Common code (arch-agnostic)                  │
│   └─────────────┘                                                   │
│        │                                                            │
│        │  Tier 1: include/drivers/*.h (Public API)                 │
│        ▼                                                            │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   Tier 2: Arch Contract                      │  │
│   │          arch/include/arch/drivers/{cpu,serial,tty}.h       │  │
│   └─────────────────────────────────────────────────────────────┘  │
│        │                              │                             │
│        ▼                              ▼                             │
│   ┌────────────────┐            ┌────────────────┐                 │
│   │ arch/x86_64/   │            │ arch/aarch64/  │   ◄── One built │
│   │ - serial.c     │            │ - serial.c     │       per arch  │
│   │ - tty.c        │            │ - tty.c        │                 │
│   │ - include/     │            │ - include/     │                 │
│   │   arch/impl/   │            │   arch/impl/   │                 │
│   │   drivers/     │            │   drivers/     │                 │
│   │   - cpu.h      │            │   - cpu.h      │  ◄── Inlined    │
│   │   - io.h       │            │   - mmio.h     │                 │
│   └────────────────┘            └────────────────┘                 │
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                arch/shared/internal/                         │  │
│   │  - framebuffer.{c,h}  (text rendering, used by both tty.c)  │  │
│   │  - terminus.h         (font data)                           │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Build System Flow

### Configuration Phase

```
┌──────────────────────────────────────────────────────────────────┐
│                         cmake configure                          │
└──────────────────────────────────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │ -DJANUS_TARGET_ARCH=? │
                    └───────────────────────┘
                                │
              ┌─────────────────┴─────────────────┐
              ▼                                   ▼
     ┌─────────────────┐                ┌─────────────────┐
     │ x86_64 (default)│                │ aarch64         │
     │                 │                │                 │
     │ HOST == TARGET  │                │ CROSS_COMPILE   │
     │ CC = gcc/clang  │                │ CC = aarch64-   │
     │ ASM = NASM      │                │      linux-gnu- │
     │ QEMU = x86_64   │                │ ASM = GAS       │
     └─────────────────┘                │ QEMU = aarch64  │
                                        └─────────────────┘
```

### Source Selection

```
kernel/
├── _start/
│   ├── common/
│   │   └── limine_requests.c     ◄── Always built (portable C)
│   │
│   ├── x86_64/                   ◄── Built when TARGET=x86_64
│   │   └── limine/
│   │       ├── entry.asm         (NASM)
│   │       └── linker.ld
│   │
│   └── aarch64/                  ◄── Built when TARGET=aarch64
│       └── limine/
│           ├── entry.S           (GAS)
│           └── linker.ld
│
└── subsys/drivers/arch/
    ├── x86_64/                   ◄── Built when TARGET=x86_64
    │   ├── serial.c
    │   ├── framebuffer.c
    │   └── tty.c
    │
    ├── aarch64/                  ◄── Built when TARGET=aarch64
    │   ├── serial.c
    │   ├── framebuffer.c
    │   └── tty.c
    │
    └── shared/                   ◄── Always available for inclusion
        └── framebuffer.c
```

### CMake Architecture Selection

```
JanusPlatform.cmake
        │
        ▼
┌───────────────────────────────────────────────────────────────────┐
│  if(NOT DEFINED JANUS_TARGET_ARCH)                                │
│      set(JANUS_TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})        │
│  endif()                                                          │
│                                                                   │
│  if(JANUS_TARGET_ARCH STREQUAL "aarch64")                        │
│      set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)                 │
│      set(JANUS_QEMU_CMD "qemu-system-aarch64")                   │
│      set(JANUS_QEMU_FLAGS "-M virt -cpu cortex-a72 ...")         │
│  else()                                                           │
│      # x86_64 defaults (native)                                   │
│      set(JANUS_QEMU_CMD "qemu-system-x86_64")                    │
│      set(JANUS_QEMU_FLAGS "-enable-kvm ...")                     │
│  endif()                                                          │
└───────────────────────────────────────────────────────────────────┘
```

---

## Execution Flow

### Boot Sequence (aarch64)

```
┌─────────────────┐
│  Power On / QEMU│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Limine (UEFI)   │   Loads kernel ELF, parses limine.conf
│                 │   Sets up initial page tables, enters kernel
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────────┐
│                        entry.S (aarch64)                         │
├─────────────────────────────────────────────────────────────────┤
│  1. Verify EL2 (Limine enters at EL2 for aarch64)              │
│  2. Configure EL2 → EL1 transition                              │
│  3. Set up initial stack pointer from limine_stack_size_request │
│  4. Zero BSS section                                            │
│  5. Load arguments (x0=magic, x1=hhdm, x2=fb_response)         │
│  6. Branch to kernel_main                                       │
└────────┬────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────────┐
│                        kernel_main()                             │
├─────────────────────────────────────────────────────────────────┤
│  1. Initialize serial (PL011 for aarch64)                       │
│  2. Initialize framebuffer (from Limine response)               │
│  3. Print boot banner                                           │
│  4. Continue kernel initialization...                           │
└─────────────────────────────────────────────────────────────────┘
```

### Driver Call Flow (Actual Implementation)

```
Application/Kernel Code
         │
         │  drivers_serial_puts("Hello")
         ▼
┌─────────────────────────────────────────────────────────────────┐
│  Tier 1: include/drivers/serial.h  (INLINED)                    │
│                                                                  │
│  static __always_inline void drivers_serial_puts(const char *s) │
│  {                                                               │
│      while (*s) {                                                │
│          while (!arch_serial_tx_ready()) { }  ◄── Compiled call │
│          arch_serial_write(*s++);             ◄── Compiled call │
│      }                                                           │
│  }                                                               │
└────────┬────────────────────────────────────────────────────────┘
         │
         │  Linker resolves arch_serial_* to:
         │
         ├─────────────── x86_64 build ───────────────┐
         │                                            │
         ▼                                            ▼
┌────────────────────────┐               ┌────────────────────────┐
│ arch/x86_64/serial.c   │               │ arch/aarch64/serial.c  │
│                        │               │                        │
│ void arch_serial_write │               │ void arch_serial_write │
│   (u8 byte) {          │               │   (u8 byte) {          │
│     outb(COM1, byte);  │               │     mmio_write32(      │
│ }                      │               │       PL011+DR, byte); │
│                        │               │ }                      │
│ bool arch_serial_      │               │                        │
│   tx_ready() {         │               │ bool arch_serial_      │
│   return inb(LINE_STAT)│               │   tx_ready() {         │
│          & TX_EMPTY;   │               │   return !(mmio_read32 │
│ }                      │               │     (PL011+FR)&TXFF);  │
│                        │               │ }                      │
└────────────────────────┘               └────────────────────────┘
       Port I/O                                  MMIO
```

---

## Driver Performance Patterns

The project uses three performance tiers based on call frequency:

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Performance-Aware Design                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  TIER A: Fully Inlined (header-only)                               │
│  ─────────────────────────────────────────────────────────────────  │
│  Used for: CPU control (halt, interrupts)                          │
│  Why: Called in hot paths, must be zero-overhead                   │
│  Pattern: static __always_inline in arch/impl/drivers/*.h          │
│                                                                     │
│    include/drivers/cpu.h                                           │
│         │  static __always_inline cpu_halt()                       │
│         ▼                                                           │
│    arch/include/arch/drivers/cpu.h (bridge)                        │
│         │  #include <arch/impl/drivers/cpu.h>                      │
│         ▼                                                           │
│    arch/x86_64/include/arch/impl/drivers/cpu.h                     │
│         │  static __always_inline arch_cpu_halt() { asm("hlt"); }  │
│                                                                     │
│  TIER B: Inlined Wrapper + Compiled Arch                           │
│  ─────────────────────────────────────────────────────────────────  │
│  Used for: Serial I/O                                              │
│  Why: Moderate frequency, loop overhead acceptable                 │
│  Pattern: Inlined public API, compiled arch_* functions            │
│                                                                     │
│    include/drivers/serial.h                                        │
│         │  static __always_inline drivers_serial_putc()            │
│         │  { while(!arch_serial_tx_ready()); arch_serial_write(); }│
│         ▼                                                           │
│    arch/x86_64/serial.c                                            │
│         │  void arch_serial_write() { outb(COM1, byte); }          │
│                                                                     │
│  TIER C: Compiled Wrapper + Compiled Arch                          │
│  ─────────────────────────────────────────────────────────────────  │
│  Used for: TTY (text display)                                      │
│  Why: Each call does significant work (draw char = 512 pixels)     │
│  Pattern: Compiled tty.c handles cursor/scroll, calls arch_tty_*   │
│                                                                     │
│    tty.c                                                           │
│         │  void drivers_tty_putc() { ...cursor...; scroll(); }     │
│         │  calls arch_tty_write_cell()                             │
│         ▼                                                           │
│    arch/x86_64/tty.c                                               │
│         │  void arch_tty_write_cell() { framebuffer_draw_char(); } │
│         ▼                                                           │
│    arch/shared/internal/framebuffer.c  (internal, not public API) │
│         │  void framebuffer_draw_char() { 512 put_pixel calls }    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Shared Framebuffer Design

### Key Insight: Framebuffer is Internal to TTY

Framebuffer rendering is NOT a public driver — it's an internal
implementation detail of TTY. This is correct because:

1. Limine provides framebuffer info in identical format on both arches
2. Pixel plotting and character drawing are purely computational
3. No arch-specific hardware access after initialization
4. Keeps public API surface minimal

```
┌─────────────────────────────────────────────────────────────────────┐
│                   Framebuffer is INTERNAL                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Public API (Tier 1)     Arch Contract (Tier 2)    Internal        │
│  ──────────────────      ──────────────────────    ────────        │
│                                                                     │
│  drivers_tty_putc()  →   arch_tty_write_cell()  →  framebuffer_*() │
│                                                                     │
│  User sees TTY only.     TTY impl decides how.     Shared code.    │
│  No framebuffer API.     VGA or framebuffer.       Both use it.    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Current x86_64 Structure (to be refactored)

```
arch/x86_64/
├── tty.c                              # arch_tty_* implementation
├── framebuffer.c                      # framebuffer_draw_char()
└── internal/arch/internal/drivers/
    ├── framebuffer.h                  # framebuffer_state_t, put_pixel
    ├── terminus.h                     # Font data
    └── vga.h                          # VGA text mode helpers
```

### Proposed Shared Structure

```
arch/
├── CMakeLists.txt                     # add_subdirectory(${JANUS_TARGET_ARCH})
│
├── include/arch/drivers/              # Tier 2: Contracts (exported PUBLIC)
│   ├── cpu.h
│   ├── serial.h
│   └── tty.h
│
├── shared/                            # Shared code (included by arch sources)
│   └── drivers/
│       ├── framebuffer.c              # framebuffer_draw_char()
│       └── include/
│           └── arch/shared/drivers/   # <arch/shared/drivers/*.h>
│               ├── framebuffer.h      # framebuffer_state_t, palette
│               └── terminus.h         # Font data (16x32 bitmap)
│
├── x86_64/
│   ├── CMakeLists.txt                 # janus_add_arch_subsys(drivers ...)
│   ├── serial.c
│   ├── tty.c                          # VGA mode OR framebuffer mode
│   ├── include/
│   │   └── arch/impl/drivers/         # <arch/impl/drivers/*.h>
│   │       ├── cpu.h
│   │       └── io.h
│   └── internal/
│       └── arch/internal/drivers/     # <arch/internal/drivers/*.h>
│           └── vga.h                  # x86-only
│
└── aarch64/
    ├── CMakeLists.txt                 # janus_add_arch_subsys(drivers ...)
    ├── serial.c                       # PL011 UART
    ├── tty.c                          # Framebuffer mode only
    └── include/
        └── arch/impl/drivers/         # <arch/impl/drivers/*.h>
            ├── cpu.h
            └── mmio.h
```

### Why This Design?

| Concern | Solution |
|---------|----------|
| Performance | Framebuffer code compiled once, linked into arch tty.c |
| Simplicity | No new public APIs, just move files |
| Extensibility | aarch64 can add cache flush in its tty.c if needed |
| No #ifdef | Build system picks arch/$(ARCH)/tty.c |

---

## Open Questions

### Q1: Arch Layer Build Structure

**Question:** How should the arch layer be built within subsystems?

**Decision:** One `CMakeLists.txt` per architecture per subsystem, using a
common helper function. Each arch file declares ALL its sources (own + shared).
The helper function handles common setup.

```
subsys/drivers/
├── CMakeLists.txt              # janus_add_subsys(drivers ...)
│                               # add_subdirectory(arch)
├── tty.c
└── arch/
    ├── CMakeLists.txt          # Dispatches to ${JANUS_TARGET_ARCH}
    ├── include/arch/drivers/   # Tier 2 contracts (exported by helper)
    ├── shared/                  # Shared sources (referenced by arch CMake)
    │   └── drivers/
    │       ├── framebuffer.c
    │       └── include/
    ├── x86_64/
    │   └── CMakeLists.txt      # janus_add_arch_subsys(drivers SOURCES ...)
    └── aarch64/
        └── CMakeLists.txt      # janus_add_arch_subsys(drivers SOURCES ...)
```

**Build Flow:**

```
subsys/drivers/CMakeLists.txt
    │
    ├── janus_add_subsys(drivers SOURCES tty.c)
    │       └── Creates: libdrivers.a
    │
    └── add_subdirectory(arch)
            │
            └── arch/CMakeLists.txt
                    │
                    └── add_subdirectory(${JANUS_TARGET_ARCH})
                            │
                            └── arch/x86_64/CMakeLists.txt  (or aarch64)
                                    │
                                    └── janus_add_arch_subsys(drivers
                                            SOURCES
                                                serial.c
                                                tty.c
                                                ../shared/drivers/framebuffer.c
                                        )
                                        └── Creates: libarch_drivers.a
```

**Helper Function Design:**

```cmake
# filepath: cmake/JanusArch.cmake
#
# janus_add_arch_subsys(<subsystem_name>
#     SOURCES file1.c file2.c ...    # All sources (own + shared)
# )
#
# - Each arch declares ALL its sources explicitly
# - Shared sources referenced via relative path: ../shared/...
# - Helper exports Tier 2 contract includes (same for all arches)
# - Helper sets up compiler flags, include paths

function(janus_add_arch_subsys name)
    cmake_parse_arguments(ARCH "" "" "SOURCES" ${ARGN})
    
    set(TARGET "arch_${name}")
    set(ARCH_BASE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    
    # Convert sources to absolute paths
    set(ABS_SOURCES "")
    foreach(src ${ARCH_SOURCES})
        if(IS_ABSOLUTE ${src})
            list(APPEND ABS_SOURCES ${src})
        else()
            list(APPEND ABS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${src})
        endif()
    endforeach()
    
    add_library(${TARGET} STATIC ${ABS_SOURCES})
    
    # PUBLIC: Tier 2 contracts (all arches share these)
    target_include_directories(${TARGET} PUBLIC
        ${ARCH_BASE}/include                     # <arch/drivers/*.h>
    )
    
    # PRIVATE: Arch-specific implementation headers
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include      # <arch/impl/drivers/*.h>
        ${CMAKE_CURRENT_SOURCE_DIR}/internal     # <arch/internal/drivers/*.h>
        ${ARCH_BASE}/shared/drivers/include      # <arch/shared/drivers/*.h>
    )
    
    target_compile_options(${TARGET} PRIVATE ${JANUS_COMMON_FLAGS})
    
    message(STATUS "Arch subsystem: ${TARGET}")
endfunction()
```

**Example: x86_64 drivers**

```cmake
# filepath: subsys/drivers/arch/x86_64/CMakeLists.txt

janus_add_arch_subsys(drivers
    SOURCES
        serial.c
        tty.c
        ../shared/drivers/framebuffer.c
)
```

**Example: aarch64 drivers**

```cmake
# filepath: subsys/drivers/arch/aarch64/CMakeLists.txt

janus_add_arch_subsys(drivers
    SOURCES
        serial.c
        tty.c
        ../shared/drivers/framebuffer.c
)
```

**Why This Design:**

| Aspect | Benefit |
|--------|--------|
| Explicit sources | Easy to see what each arch builds |
| Shared via path | No special "SHARED" parameter needed |
| One CMake per arch | Clean separation, easy to add new arch |
| Helper function | Common setup, less boilerplate |
| Public includes in helper | Tier 2 contracts exported once |

---

### Q2: QEMU Machine Type for aarch64

**Question:** Which QEMU machine should be the default?

| Machine      | Pros                           | Cons                        |
|--------------|--------------------------------|-----------------------------|
| `-M virt`    | Clean, fast, good QEMU support | Not real hardware           |
| `-M raspi3b` | Matches target hardware        | Slower, less QEMU features  |

**Decision:** Use `-M virt -cpu cortex-a72` exclusively for initial
implementation. The virt machine has consistent, well-documented
peripherals (PL011 at 0x09000000) and avoids the complexity of
board-specific memory maps.

Real Raspberry Pi testing will be added in a later phase alongside
Device Tree support, which enables runtime discovery of peripheral
addresses.

---

### Q3: EL2 vs EL1 Entry

**Question:** Limine enters the kernel at EL2 on aarch64. Should we:

A) Drop to EL1 immediately in entry.S  
B) Stay at EL2 and implement a hypervisor-capable kernel  
C) Make it configurable  

**Recommendation:** Option A (drop to EL1). A hobby OS doesn't need
hypervisor features, and EL1 is simpler to reason about. Leave EL2
support as a future enhancement.

---

### Q4: Multiboot2 Fate

**Question:** Currently there's `kernel/_start/x86_64/multiboot2/`.
Should we:

A) Keep it (x86_64 only, two bootloader options)  
B) Remove it (simplify to Limine only)  
C) Keep it but mark as deprecated  

**Recommendation:** Option C for now. Don't invest effort in multiboot2
aarch64 (it's x86-specific anyway). Focus on Limine. Consider removal
in a future cleanup pass.

---

### Q5: UART Base Address Handling

**Question:** PL011 UART address differs between platforms:

- QEMU virt: 0x09000000
- Raspberry Pi 3B: 0x3F201000
- Raspberry Pi 4: 0xFE201000

How should we handle this?

**Option A: Compile-time #define**

```c
#if defined(JANUS_PLATFORM_QEMU_VIRT)
#define PL011_BASE 0x09000000
#elif defined(JANUS_PLATFORM_RASPI3)
#define PL011_BASE 0x3F201000
#endif
```

**Option B: Runtime detection via DTB**
Limine can provide device tree blob. Parse it to find UART address.

**Option C: Board-specific source files**

```
arch/aarch64/
├── serial.c           # Common PL011 driver
├── boards/
│   ├── qemu_virt.c    # Sets PL011_BASE = 0x09000000
│   └── raspi3.c       # Sets PL011_BASE = 0x3F201000
```

**Decision:** For initial implementation, hardcode QEMU virt address
directly (0x09000000). No conditional compilation needed since we only
target one platform.

When adding real hardware support, implement **Option B (DTB parsing)**.
This aligns with the goal of avoiding `#ifdef` in C code and enables
a single kernel binary for all aarch64 boards. The DTB approach is
the industry standard (Linux, FreeBSD) and future-proofs the design.

---

## Implementation Order

```
Phase 1: Minimal Boot (Todos 1, 2, 3, 4, 5)
──────────────────────────────────────────
Goal: Boot on QEMU aarch64 and print "Hello" via serial

    [1] Build System ──► [2] Entry Point ──► [3] Limine Verify
                                   │
                                   ▼
                            [4] CPU Driver (inlined)
                                   │
                                   ▼
                            [5] PL011 Serial
                                   │
                                   ▼
                         "Hello from aarch64!"


Phase 2: Display Output (Todos 6, 7)
──────────────────────────────────────────
Goal: Graphical output via framebuffer

    [6] TTY + Shared Framebuffer ──► [7] Verify x86_64 Still Works
                                   │
                                   ▼
                         Boot banner on screen (both arches)


Phase 3: Polish (Todos 8, 9, 10)
──────────────────────────────────────────
Goal: Unified developer experience

    [8] QEMU Target ──► [9] Unified Build ──► [10] Documentation
                                   │
                                   ▼
                         ninja run "just works"


Phase 4: Real Hardware (Todo 11) [FUTURE]
──────────────────────────────────────────
Goal: Boot on actual Raspberry Pi hardware

    [11] DTB Parsing ──► Peripheral Discovery ──► Pi 3B/4 Testing
                                   │
                                   ▼
                         Single binary, multiple boards
```

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Limine aarch64 bugs | Low | High | Test early, report upstream, have workarounds |
| GAS assembly learning curve | Medium | Low | Reference existing aarch64 kernels |
| QEMU aarch64 quirks | Medium | Medium | Use `-M virt` primarily, document issues |
| Cross-toolchain setup | Low | Medium | Document exact package names per distro |
| Cache coherency issues | Medium | High | Keep it simple, avoid fancy caching initially |
| Different calling conventions | Low | Medium | Let compiler handle it, minimal inline asm |

---

## References

- Limine Protocol: <https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md>
- ARM64 Calling Convention: AAPCS64
- BCM2837 Peripherals: <https://datasheets.raspberrypi.com/bcm2837/bcm2837-peripherals.pdf>
- PL011 UART: ARM PrimeCell UART (PL011) Technical Reference Manual
- OSDev ARM64: <https://wiki.osdev.org/ARM_Overview>

---

## Appendix: Directory Structure After Implementation

```
kernel/
├── _start/
│   ├── common/
│   │   └── limine_requests.c              ✓ Exists (portable)
│   │
│   ├── x86_64/
│   │   └── limine/
│   │       ├── entry.asm                  ✓ Exists
│   │       └── linker.ld                  ✓ Exists
│   │
│   └── aarch64/                           NEW
│       └── limine/
│           ├── entry.S
│           └── linker.ld
│
├── subsys/
│   └── drivers/
│       ├── CMakeLists.txt                 # janus_add_subsys + add_subdirectory(arch)
│       ├── tty.c                          ✓ Exists (cursor/scroll)
│       │
│       ├── include/drivers/               ✓ Tier 1: Public API
│       │   ├── cpu.h                      (header-only, inlined)
│       │   ├── serial.h                   (inlined wrappers)
│       │   └── tty.h                      (compiled wrapper)
│       │
│       └── arch/
│           ├── CMakeLists.txt             # add_subdirectory(${JANUS_TARGET_ARCH})
│           │
│           ├── include/arch/drivers/      ✓ Tier 2: Arch Contract
│           │   ├── cpu.h                  (bridge → arch/impl/)
│           │   ├── serial.h               ✓ Exists
│           │   └── tty.h                  ✓ Exists
│           │
│           ├── shared/                    NEW
│           │   └── drivers/
│           │       ├── framebuffer.c      (draw_char)
│           │       └── include/
│           │           └── arch/shared/drivers/
│           │               ├── framebuffer.h  (state, palette, put_pixel)
│           │               └── terminus.h     (font data)
│           │
│           ├── x86_64/
│           │   ├── CMakeLists.txt         # janus_add_arch_subsys(drivers ...)
│           │   ├── serial.c               ✓ Exists (16550 UART)
│           │   ├── tty.c                  ✓ Exists (VGA + FB modes)
│           │   ├── include/arch/impl/drivers/
│           │   │   ├── cpu.h              ✓ Exists (hlt, cli, sti)
│           │   │   └── io.h               ✓ Exists (inb, outb)
│           │   └── internal/arch/internal/drivers/
│           │       └── vga.h              ✓ Exists (x86 VGA only)
│           │
│           └── aarch64/                   NEW
│               ├── CMakeLists.txt         # janus_add_arch_subsys(drivers ...)
│               ├── serial.c               (PL011, 0x09000000)
│               ├── tty.c                  (framebuffer only)
│               └── include/arch/impl/drivers/
│                   ├── cpu.h              (wfi, msr daif)
│                   └── mmio.h             (mmio_read/write)
│
└── include/janus/
    ├── types.h                            ✓ Exists
    └── attributes.h                       ✓ Exists (__always_inline)
```

To fix serial I need to:

Map the UART MMIO region in your page tables
Or use Limine's terminal request feature for early output

framebuffer values: aarch64
(tty_display_config_t) config = {
  framebuffer = 0xffff00004ecc0000
  width = 800
  height = 600
  pitch = 3200
  bpp = 32
  red_mask_shift = '\x10'
  green_mask_shift = '\b'
  blue_mask_shift = '\0'
}

framebuffer values x86
config = {
  framebuffer = 0xffff8000fd000000
  width = 1280
  height = 800
  pitch = 5120
  bpp = 32
  red_mask_shift = '\x10'
  green_mask_shift = '\b'
  blue_mask_shift = '\0'
}
