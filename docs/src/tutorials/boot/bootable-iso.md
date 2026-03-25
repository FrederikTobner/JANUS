# Creating a Bootable ISO

This tutorial walks through turning a bare-metal kernel ELF into a bootable ISO image using GRUB and `grub-mkrescue`. The resulting image boots in QEMU, VirtualBox, or on real BIOS/UEFI hardware. We also set up CMake targets so that building, running, and debugging are single commands.

## Prerequisites

A working kernel ELF that speaks Multiboot2 (has a valid header and a `_start` entry point). The [Multiboot2](../../wiki/boot/multiboot2.md) concept page covers the protocol. The following host tools must be installed:

- `grub-mkrescue` (package `grub-pc-bin` on Debian, `grub` on Arch)
- `xorriso` — the back-end that `grub-mkrescue` uses to create the ISO 9660 image
- `mtools` — provides FAT filesystem utilities needed for the EFI boot partition

## Why an ISO?

QEMU's `-kernel` flag can boot a Linux-style bzImage or a PVH kernel, but a Multiboot2 kernel is neither. It needs a Multiboot2-aware bootloader to load it. GRUB fills that role. Packaging GRUB and the kernel together into an ISO gives us a single file that works everywhere — emulators, USB sticks, and CD drives.

[!side]
ISO 9660 is the CD-ROM filesystem standard from 1988. Despite its age, it remains the universal format for bootable images because every BIOS, UEFI firmware, and virtualiser can read it.
[/!side]

## GRUB Configuration

GRUB reads its configuration from `grub.cfg`. For a Multiboot2 kernel the configuration is minimal:

```cfg
set timeout=0
set default=0

menuentry "JANUS" {
    multiboot2 /boot/kernel.elf
    boot
}
```

- `timeout=0` skips the menu and boots immediately.
- `multiboot2` tells GRUB to load the file using the Multiboot2 protocol (not as a Linux kernel).
- The path `/boot/kernel.elf` is relative to the root of the ISO filesystem.

In a CMake build system, store this as a template (`grub.cfg.in`) and copy it into the build directory with `configure_file`:

```cmake
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/boot/multiboot2/grub.cfg.in
    ${CMAKE_BINARY_DIR}/grub.cfg
    @ONLY
)
```

## Building the ISO

GRUB expects a specific directory structure inside the ISO:

```
iso/
└── boot/
    ├── kernel.elf
    └── grub/
        └── grub.cfg
```

We define a CMake custom target that assembles this structure and calls `grub-mkrescue`:

```cmake
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf>
            ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg
            ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/janus.iso ${CMAKE_BINARY_DIR}/iso/
    DEPENDS kernel.elf ${CMAKE_BINARY_DIR}/grub.cfg
    COMMENT "Creating bootable ISO image"
)
```

Build it:

```bash
ninja -C build iso
```

`grub-mkrescue` embeds both a BIOS and an EFI bootloader into the image, so the same `.iso` works on both firmware types.

## Run and Debug Targets

Two additional CMake targets turn "build → run → debug" into single commands:

```cmake
add_custom_target(run
    COMMAND qemu-system-x86_64
        -cdrom ${CMAKE_BINARY_DIR}/janus.iso
        -boot d -serial stdio
    DEPENDS iso
    USES_TERMINAL
    COMMENT "Running JANUS in QEMU"
)

add_custom_target(debug
    COMMAND qemu-system-x86_64
        -cdrom ${CMAKE_BINARY_DIR}/janus.iso
        -boot d -serial stdio -s -S
    DEPENDS iso
    USES_TERMINAL
    COMMENT "Running JANUS in QEMU (debugger on :1234)"
)
```

- `USES_TERMINAL` ensures `ninja run` shows QEMU's serial output in the build terminal instead of buffering it.
- `-serial stdio` redirects the virtual COM1 to the host terminal — see [Serial Port Driver](../io/serial-port-driver.md).
- `-s -S` enables the GDB stub and pauses the CPU at startup — see [Debugging a Kernel with LLDB](../debugging/kernel-debugging-lldb.md).

Usage:

```bash
ninja -C build run      # build + boot
ninja -C build debug    # build + boot, paused for debugger
```

## Key Takeaways

- **GRUB bridges the Multiboot2 gap.** QEMU cannot boot a Multiboot2 ELF directly; GRUB validates the header, loads the kernel, and enters it in 32-bit protected mode.
- **`grub-mkrescue` produces a hybrid image.** The same ISO boots via BIOS (El Torito) and UEFI (embedded FAT EFI System Partition).
- **CMake custom targets chain dependencies.** `run` depends on `iso`, which depends on `kernel.elf` — one command triggers the full pipeline.
