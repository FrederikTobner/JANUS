# Firmware

Firmware is the first software that runs after a computer is powered on. It initialises the hardware, like memory controllers, buses, storage devices, and provides a standardised interface through which a bootloader or operating system can discover and configure the machine.
Two families of firmware dominate the market.
The older BIOS and the more modern UEFI.

## BIOS

The Basic Input/Output System is the original PC firmware standard, dating back to the IBM PC in 1981.
When a BIOS machine powers on, the processor begins executing at a hardcoded address in real mode (16-bit), and the BIOS performs a Power-On Self-Test (POST), initialises hardware, discovers bootable media, and loads the first 512 bytes — the Master Boot Record — from the selected disk into memory at address `0x7C00`.

BIOS provides a set of interrupt-based services (INT 10h for video, INT 13h for disk access) that bootloaders can use while still in real mode. These services are unavailable once the processor transitions to 32-bit protected or 64-bit long mode, which is why BIOS-era bootloaders must complete all firmware interaction before switching CPU modes.

The limitations of BIOS are significant: it can only boot from MBR-partitioned disks, it has no native support for partitions larger than 2 TiB, its boot sector is limited to 512 bytes, and its services are 16-bit only. These constraints drove the development of its successor.

## UEFI

The Unified Extensible Firmware Interface is the modern replacement for BIOS, developed originally by Intel and now maintained by the UEFI Forum. UEFI boots in 32-bit or 64-bit mode (no real mode involved), reads GPT-partitioned disks natively, and supports a FAT32-formatted EFI System Partition from which it loads bootloader executables in PE format.

Unlike BIOS, UEFI provides a rich runtime environment with boot services (memory allocation, timer access, block I/O, graphics output) and runtime services (variable storage, clock access) that remain available even after the operating system has started. Before calling `ExitBootServices()`, the bootloader can use UEFI facilities to allocate memory, query the memory map, and set the framebuffer mode.

Most modern machines, nowadays ship with UEFI firmware instead of a BIOS.
Many also include a Compatibility Support Module (CSM) that emulates BIOS for legacy operating systems, though CSM support is increasingly being dropped by vendors.

## Implications for OS Development

[!side]
See [Bootloaders](bootloaders.md) for how bootloaders build on firmware services to load and configure the kernel.
[/!side]

An OS kernel typically does not interact with firmware directly. This is responsibility of the bootloader.
However, the firmware determines what information is available at boot time: the memory map format, the framebuffer setup mechanism, and the address at which the kernel is loaded. Boot protocols like [Multiboot2](multiboot2.md) and Limine abstract over these firmware differences, providing a uniform interface that the kernel can consume regardless of whether the underlying firmware is BIOS or UEFI.
