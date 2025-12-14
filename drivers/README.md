# drivers - Device Drivers Module

Kernel device drivers for hardware interaction.

## Purpose

Provides device drivers for keyboards, storage, network cards, and other hardware peripherals.

## Planned Contents

### Input Devices
- PS/2 keyboard driver
- PS/2 mouse driver
- USB HID support (future)

### Storage Devices
- ATA/IDE disk driver
- AHCI SATA driver
- NVMe driver (future)

### Network Devices
- Intel E1000 network driver
- RTL8139 network driver (future)

### Other Devices
- PCI/PCIe bus enumeration
- ACPI power management
- RTC (Real-Time Clock)

## Structure

```
drivers/
├── keyboard/
│   └── ps2_keyboard.c
├── storage/
│   ├── ata.c
│   └── ahci.c
├── network/
│   └── e1000.c
├── pci/
│   └── pci.c
└── include/drivers/
    ├── keyboard.h
    ├── storage.h
    └── network.h
```

## Dependencies

- `types` - Type definitions
- `arch` - I/O operations
- `fio` - Debug output
- `mm` - Memory allocation

## Status

⚠️ **Placeholder** - Implementation pending in future development phases.
