# Limine Boot Protocol Support Plan

## Goal

Add Limine boot protocol support to JANUS while maintaining Multiboot2 compatibility, shipping a single ISO per architecture that supports both boot protocols.

## Background

### Current State

- `_start/x86_64/multiboot2/` contains entry point, linker script, and protocol header
- ISO is created with GRUB as bootloader
- Single entry point per architecture

### Limine vs Multiboot2

| Aspect | Multiboot2 | Limine |
|--------|------------|--------|
| Bootloader | GRUB | Limine |
| Protocol complexity | Medium | Simple, modern |
| Higher-half support | Manual setup | Built-in |
| Framebuffer | Request via tags | Request via protocol |
| Memory map | Tags | Clean struct |

## Architecture Options

### Option A: Separate Kernel Binaries, Single ISO

```
kernel/
└── _start/
    └── x86_64/
        ├── multiboot2/           # → kernel-mb2.elf
        │   ├── boot.asm
        │   ├── multiboot2.asm
        │   └── link.ld
        └── limine/               # → kernel-limine.elf
            ├── boot.asm
            └── link.ld
```

**How it works:**

- Build two separate kernel ELF files
- ISO contains both + both bootloader configs
- Limine bootloader can chainload GRUB or boot directly
- User selects at boot time

**Pros:**

- Clean separation, no ifdef/runtime detection
- Each protocol optimally implemented
- Easy to test each independently

**Cons:**

- Two build artifacts to manage
- Duplicated kernel code in ISO (larger ISO)
- CMake complexity for two targets

---

### Option B: Single Kernel with Multiple Entry Points

```
kernel/
└── _start/
    └── x86_64/
        ├── common/               # Shared early init
        │   └── early_init.asm
        ├── multiboot2/
        │   ├── entry.asm         # _start_mb2
        │   └── header.asm
        ├── limine/
        │   └── entry.asm         # _start_limine
        └── link.ld               # Single linker script, multiple entry symbols
```

**How it works:**

- Single kernel.elf with multiple entry point symbols
- Linker script exports both `_start_mb2` and `_start_limine`
- Bootloader config specifies which entry to use
- Both entries converge to shared `kmain`

**Pros:**

- Single kernel binary
- Shared code maximized
- Smaller ISO

**Cons:**

- More complex linker script
- Both protocol handlers always linked in
- Potential symbol conflicts

---

### Option C: Arch Layer Pattern in _start (Recommended)

Apply the established three-tier arch pattern to `_start`:

```
kernel/
└── _start/
    ├── include/_start/           # Tier 1: Common entry API
    │   └── entry.h               # Declares kernel_entry(boot_info*)
    ├── arch/
    │   ├── include/arch/_start/  # Tier 2: Arch-specific boot info
    │   │   └── boot_info.h       # Unified boot_info structure
    │   └── x86_64/
    │       ├── include/arch/impl/_start/
    │       │   └── boot_info.h   # x86_64 boot_info details
    │       ├── common/           # Shared x86_64 startup
    │       │   ├── gdt.asm
    │       │   └── early_init.asm
    │       ├── multiboot2/
    │       │   ├── entry.asm
    │       │   ├── header.asm
    │       │   └── parse.c       # Parse MB2 → boot_info
    │       └── limine/
    │           ├── entry.asm
    │           └── parse.c       # Parse Limine → boot_info
    └── link/
        └── x86_64.ld             # Shared linker script
```

**How it works:**

- Both protocols parse their data into a unified `boot_info` structure
- Shared early init code (GDT, paging setup) used by both
- Single kernel.elf, multiple entry points
- Protocol-specific code is minimal (just parsing)

**Pros:**

- Consistent with existing arch layer patterns
- Maximizes code reuse
- Clean abstraction (protocol details hidden from kernel)
- Extensible to other protocols (UEFI stub, etc.)

**Cons:**

- More directories/files
- Need to design unified boot_info structure

---

### Option D: Protocol as Build-Time Configuration

```
kernel/
└── _start/
    └── x86_64/
        ├── entry.asm.in          # Template with @PROTOCOL@ markers
        ├── multiboot2/
        │   └── protocol.cmake    # Defines protocol-specific vars
        └── limine/
            └── protocol.cmake
```

**How it works:**

- CMake selects protocol at configure time
- Templates generate protocol-specific code
- Build once per protocol, combine ISOs

**Pros:**

- Simple runtime (no dead code)
- Clear build configuration

**Cons:**

- Not a single kernel binary
- Less flexible than runtime multi-protocol

---

## ISO Structure Options

### Option I: Limine as Primary Bootloader

```
iso/
├── boot/
│   └── limine/
│       ├── limine.cfg            # Limine config
│       └── limine-bios.sys       # Limine BIOS bootloader
├── EFI/
│   └── BOOT/
│       └── BOOTX64.EFI           # Limine UEFI
├── kernel.elf                    # Or kernel-mb2.elf + kernel-limine.elf
└── limine.cfg
```

**Limine config example:**

```
TIMEOUT=5

:JANUS (Limine)
    PROTOCOL=limine
    KERNEL_PATH=boot:///kernel.elf
    KERNEL_CMDLINE=console=tty0

:JANUS (Multiboot2)
    PROTOCOL=multiboot2
    KERNEL_PATH=boot:///kernel.elf
```

**Note:** Limine can load Multiboot2 kernels directly! This may simplify things significantly.

### Option II: Dual Bootloader ISO

```
iso/
├── boot/
│   ├── grub/
│   │   └── grub.cfg
│   └── limine/
│       └── limine.cfg
├── kernel.elf
└── ...
```

**Cons:** More complex ISO creation, potential conflicts

---

## Chosen Approach

**Decisions:**

- **Option C** (arch layer pattern) for `_start` organization
- **Option II** (dual bootloader ISO) to keep both GRUB and Limine
- **Single `kernel.elf`** with multiple entry points (smaller ISO)
- **No `boot_info` abstraction** - use existing magic + void* pattern
- **Higher-half kernel from the start** - both protocols load kernel at high addresses
- **Bootloader handles UEFI** - no direct UEFI stub needed
- **System-installed Limine** - no vendored binaries

### Higher-Half Kernel Design

The kernel is linked at virtual address `0xFFFFFFFF80000000` (higher-half). Both boot protocols must ensure the kernel runs at this address.

**Virtual Memory Layout:**

```
0xFFFFFFFF80000000+  ┌─────────────────────┐
                     │   Kernel (.text)    │  ← kernel runs here
                     │   Kernel (.data)    │
                     │   Kernel (.bss)     │
                     └─────────────────────┘
        ...          │   (unmapped)        │
0x0000000000400000   ┌─────────────────────┐
                     │   User space        │  ← future user programs
                     └─────────────────────┘
0x0000000000000000   │   (unmapped/null)   │
```

**Protocol-specific handling:**

| Protocol | Higher-Half Support |
|----------|---------------------|
| Limine | Built-in! Request `LIMINE_HIGHER_HALF` and kernel starts at high address |
| Multiboot2 | Manual. Entry must set up page tables before jumping to high address |

**Multiboot2 entry flow (more complex):**

```asm
_start_multiboot2:          ; Running at physical ~1MB, identity-mapped
    ; 1. Save magic + pointer
    ; 2. Set up page tables:
    ;    - Identity map low memory (temporary)
    ;    - Map 0xFFFFFFFF80000000 → physical kernel
    ; 3. Enable paging (CR3, CR0.PG)
    ; 4. Jump to higher-half address
    jmp _start_multiboot2_high

_start_multiboot2_high:     ; Now running at 0xFFFFFFFF80xxxxxx
    ; 5. Remove identity mapping (optional, for cleanliness)
    ; 6. Call kmain(magic, ptr)
```

**Limine entry flow (simple):**

```asm
_start_limine:              ; Already running at 0xFFFFFFFF80xxxxxx
    ; Limine set up paging for us!
    ; Just call kmain(magic, ptr)
```

### Final `_start` Structure

```
kernel/
└── _start/
    └── x86_64/
        ├── common/                   # Shared x86_64 startup code
        │   ├── gdt.asm               # GDT setup (shared)
        │   └── stack.asm             # Stack setup (shared)
        ├── multiboot2/
        │   ├── entry.asm             # _start_multiboot2 (sets up paging!)
        │   ├── header.asm            # Multiboot2 header
        │   └── paging.asm            # Page table setup for higher-half
        ├── limine/
        │   ├── entry.asm             # _start_limine (paging done by Limine)
        │   └── requests.asm          # Limine request structures
        └── link.ld                   # Higher-half linker script
```

**Key points:**

- Each protocol has its own entry point symbol (`_start_multiboot2`, `_start_limine`)
- Both entry points call `kmain(magic, boot_data_ptr)` directly
- No parsing in `_start` - that happens in `boot` subsystem
- Single linker script handles both, with protocol-specific sections

### Single ELF with Multiple Entry Points

```
kernel.elf
├── .multiboot2_header            # Multiboot2 header (GRUB looks for this)
├── .limine_requests              # Limine request structures
├── .text.multiboot2_entry        # _start_multiboot2
├── .text.limine_entry            # _start_limine
├── .text                         # Shared kernel code
└── ...
```

**Linker script excerpt (higher-half):**

```ld
ENTRY(_start_multiboot2)  /* Default entry for direct QEMU boot */

/* Higher-half virtual base */
KERNEL_VMA = 0xFFFFFFFF80000000;
KERNEL_LMA = 0x100000;  /* Physical load address (1MB) */

SECTIONS {
    /* Multiboot2 header must be in first 32KB, at physical address */
    . = KERNEL_LMA;
    .multiboot2_header : { *(.multiboot2_header) }
    
    /* Kernel code/data at higher-half virtual address */
    . = KERNEL_VMA + SIZEOF(.multiboot2_header);
    
    .limine_requests : AT(ADDR(.limine_requests) - KERNEL_VMA + KERNEL_LMA) {
        *(.limine_requests)
    }
    
    .text : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA) {
        *(.text.multiboot2_entry)
        *(.text.limine_entry)
        *(.text .text.*)
    }
    
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA) {
        *(.rodata .rodata.*)
    }
    
    .data : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA) {
        *(.data .data.*)
    }
    
    .bss : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA) {
        *(.bss .bss.*)
    }
}
```

**Key points:**

- `AT()` specifies physical load address (LMA)
- Virtual addresses (VMA) are in higher-half
- Multiboot2 header must stay at low physical address (GRUB requirement)

### Final ISO Structure

```
iso/
├── boot/
│   ├── grub/
│   │   └── grub.cfg              # GRUB config (uses _start_multiboot2)
│   └── limine/
│       ├── limine.cfg            # Limine config (can use either entry)
│       └── limine-bios.sys
├── EFI/
│   └── BOOT/
│       └── BOOTX64.EFI           # Limine UEFI bootloader
├── kernel.elf                    # Single kernel with both protocols
└── limine.cfg                    # Root config for Limine
```

**grub.cfg:**

```
menuentry "JANUS" {
    multiboot2 /kernel.elf
    boot
}
```

**limine.cfg:**

```
TIMEOUT=5

:JANUS (Limine Protocol)
    PROTOCOL=limine
    KERNEL_PATH=boot:///kernel.elf
    ENTRY_POINT=_start_limine

:JANUS (Multiboot2 Protocol)
    PROTOCOL=multiboot2
    KERNEL_PATH=boot:///kernel.elf
```

### Boot Flow (Higher-Half)

```
┌─────────────────────────────────────────────────────────────────┐
│                        BIOS/UEFI                                │
└─────────────────────────┬───────────────────────────────────────┘
                          │
          ┌───────────────┴───────────────┐
          ▼                               ▼
    ┌───────────┐                   ┌───────────┐
    │   GRUB    │                   │  Limine   │
    └─────┬─────┘                   └─────┬─────┘
          │                               │
          │ Multiboot2                    │ Limine protocol
          │ (loads at 1MB physical)       │ (sets up higher-half)
          ▼                               ▼
┌─────────────────────────┐     ┌─────────────────────────┐
│  _start_multiboot2      │     │    _start_limine        │
│  (physical ~1MB)        │     │  (already at 0xFFFF...) │
│                         │     │                         │
│  1. Save magic+ptr      │     │  1. Get magic+ptr       │
│  2. Setup page tables   │     │  2. Call kmain()        │
│  3. Enable paging       │     │     (that's it!)        │
│  4. Jump to high addr   │     │                         │
│  5. Call kmain()        │     │                         │
└────────────┬────────────┘     └────────────┬────────────┘
             │                               │
             │  magic=MB2_MAGIC              │  magic=LIMINE_MAGIC
             │  ptr=multiboot_info           │  ptr=limine_response
             │  (running at 0xFFFF...)       │  (running at 0xFFFF...)
             │                               │
             └───────────────┬───────────────┘
                             ▼
                  ┌─────────────────────┐
                  │  kmain(magic, ptr)  │
                  │  @ 0xFFFFFFFF80...  │
                  └──────────┬──────────┘
                             ▼
                  ┌─────────────────────┐
                  │   boot subsystem    │
                  │ (checks magic,      │
                  │  parses protocol)   │
                  └─────────────────────┘
```

---

## Implementation Plan

### Phase 1: Restructure `_start`

1. Create `_start/x86_64/common/` for shared early init (GDT, stack)
2. Move current multiboot2 code to `_start/x86_64/multiboot2/`
3. Rename entry symbol to `_start_multiboot2`
4. Create higher-half `link.ld` with VMA/LMA separation

### Phase 2: Multiboot2 Higher-Half Support

1. Create `_start/x86_64/multiboot2/paging.asm` - page table setup
2. Update `entry.asm` to:
   - Set up identity mapping + higher-half mapping
   - Enable paging
   - Jump to higher-half address
   - Call kmain

### Phase 3: Add Limine Protocol

1. Create `_start/x86_64/limine/entry.asm`
2. Create `_start/x86_64/limine/requests.asm` (include `LIMINE_HIGHER_HALF` request!)
3. Define `_start_limine` entry point
4. Add Limine magic constant to kernel

### Phase 3: Update `boot` Subsystem

1. Add Limine magic detection
2. Add Limine response parsing (similar to multiboot2)
3. Extract memory map, framebuffer, etc.

### Phase 4: Dual Bootloader ISO

1. Update CMake ISO target to include both bootloaders
2. Add limine.cfg generation
3. Install Limine binaries to ISO
4. Test both boot paths

### Phase 5: CMake Cleanup

1. Update `_start` CMakeLists.txt for new structure
2. Add `JANUS_BOOT_PROTOCOLS` configuration
3. Add separate `iso-grub` and `iso-limine` targets (optional)

---

## CMake Changes

### New Variables

```cmake
JANUS_BOOT_PROTOCOLS      # List: "multiboot2;limine" (default: both)
```

### Updated Targets

```cmake
kernel.elf                # Single kernel with all enabled protocols
iso                       # Creates dual-bootloader ISO
run                       # Boot ISO in QEMU (select protocol from boot menu)
run-elf                   # Direct QEMU multiboot load (fast, Multiboot2 only)
debug                     # Boot ISO with GDB server (select protocol from menu)
debug-elf                 # Direct QEMU multiboot with GDB server
```

### `_start` CMakeLists.txt Structure

```cmake
# Common x86_64 startup code
add_library(_start_common OBJECT
    x86_64/common/gdt.asm
    x86_64/common/stack.asm
)

# Protocol-specific entry points
if("multiboot2" IN_LIST JANUS_BOOT_PROTOCOLS)
    add_library(_start_multiboot2 OBJECT
        x86_64/multiboot2/entry.asm
        x86_64/multiboot2/header.asm
        x86_64/multiboot2/paging.asm
    )
endif()

if("limine" IN_LIST JANUS_BOOT_PROTOCOLS)
    add_library(_start_limine OBJECT
        x86_64/limine/entry.asm
        x86_64/limine/requests.asm
    )
endif()

# Link all into kernel.elf
janus_link_kernel(kernel.elf
    OBJECTS _start_common _start_multiboot2 _start_limine kmain
    LIBRARIES drivers boot
    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/x86_64/link.ld
)
```

---

## Testing

### Quick Testing Commands

```bash
# Build
cmake -B build -G Ninja
ninja -C build

# Boot ISO - select Multiboot2 or Limine from GRUB menu
ninja -C build run

# UEFI boot (requires OVMF installed)
ninja -C build run-uefi

# Debug variants (starts GDB server on :1234)
ninja -C build debug
```

**Note:** QEMU's `-kernel` flag only supports Multiboot1, not Multiboot2, so direct kernel
loading without an ISO is not available. Always use `ninja run` which boots via ISO.

### Testing Workflow

1. **Test Multiboot2** → Use `ninja run`, select "JANUS" from GRUB menu
2. **Test Limine** → When Limine is installed and ISO is created with it, select from menu
3. **Test UEFI** → Use `ninja run-uefi` (requires OVMF)

### What to Verify

When testing each boot protocol, verify:

1. **Boot succeeds** - Kernel reaches `kmain()`
2. **Magic value correct** - `kmain` receives expected magic:
   - Multiboot2: `0x36d76289`
   - Limine: `0xEDFE0DD0EDFE0DD0` (or similar, check Limine spec)
3. **Boot data accessible** - Memory map, framebuffer info parsed correctly
4. **Higher-half working** - Kernel running at `0xFFFFFFFF80...` addresses
5. **Serial output works** - Debug messages appear on console

### Test Matrix

| Test | GRUB/MB2 | Limine BIOS | Limine UEFI |
|------|----------|-------------|-------------|
| Boot to kmain | ⬜ | ⬜ | ⬜ |
| Magic detection | ⬜ | ⬜ | ⬜ |
| Memory map | ⬜ | ⬜ | ⬜ |
| Framebuffer | ⬜ | ⬜ | ⬜ |
| Higher-half | ⬜ | ⬜ | ⬜ |
| Serial output | ⬜ | ⬜ | ⬜ |

### CMake Implementation for Run/Debug Targets

```cmake
# ISO boot targets
add_custom_target(run
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
        -boot d -serial stdio -m 256M ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Boots JANUS ISO (select protocol from boot menu)..."
    USES_TERMINAL
)

add_custom_target(debug
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
        -boot d -serial stdio -m 256M -s -S ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Boots JANUS ISO with GDB server on :1234..."
    USES_TERMINAL
)

# UEFI boot (requires OVMF)
add_custom_target(run-uefi
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}
        -boot d -serial stdio -m 256M ${QEMU_DISPLAY_ARG}
        -bios /usr/share/OVMF/OVMF_CODE.fd
    DEPENDS iso
    COMMENT "Boots JANUS ISO in UEFI mode (requires OVMF)..."
    USES_TERMINAL
)
```

**Note:** QEMU's `-kernel` only supports Multiboot1, not Multiboot2, so the `run-elf` and
`debug-elf` targets are not functional. Always use ISO boot with `ninja run`.

**Usage summary:**

| Command | Description |
|---------|-------------|
| `ninja run` | Boot ISO, select protocol from GRUB menu |
| `ninja debug` | Boot ISO with GDB server on :1234 |
| `ninja run-uefi` | Boot ISO in UEFI mode (requires OVMF) |

---

## Open Questions

None! All major decisions have been made:

- ✅ Option C (arch layer pattern) + Option II (dual bootloader)
- ✅ Single kernel.elf with multiple entry points
- ✅ Higher-half kernel from the start
- ✅ Bootloader handles UEFI (no direct stub)
- ✅ System-installed Limine

---

## References

- [Limine Boot Protocol Specification](https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md)
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
- [OSDev Wiki: Limine](https://wiki.osdev.org/Limine)
