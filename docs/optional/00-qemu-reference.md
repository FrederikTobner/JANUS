# QEMU Reference

Cause I would loose me head if it would not be attached to my shoulders here is an overview of QEMU.

## Invocation

The `run-<protocol>` and `debug-<protocol>` CMake targets invoke QEMU with the correct flags for the selected preset.
Direct invocation is rarely necessary, but the underlying commands are:

```bash
# x86_64 Limine ISO
qemu-system-x86_64 -cdrom build-x86_64-gcc/janus_x86_64.iso -serial stdio -m 256M

# aarch64 Limine
qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel build-aarch64-gcc/kernel-limine.elf \
    -serial stdio -m 256M
```

## Useful Flags

| Flag                                 | Effect                                                       |
|--------------------------------------|--------------------------------------------------------------|
| `-serial stdio`                      | Redirect the virtual serial port to the host terminal        |
| `-m 256M`                            | Set guest RAM size                                           |
| `-no-reboot`                         | Exit the VM instead of rebooting on triple fault             |
| `-no-shutdown`                       | Keep QEMU open after the guest halts                         |
| `-d int,cpu_reset`                   | Log all interrupts and CPU resets to stderr                  |
| `-D qemu.log`                        | Write debug log to a file instead of stderr                  |
| `-s`                                 | Start a GDB server on `localhost:1234`                       |
| `-S`                                 | Freeze the CPU at startup and wait for a debugger connection |
| `-bios /usr/share/OVMF/OVMF_CODE.fd` | Boot in UEFI mode (x86_64)                                   |

## QEMU Monitor

The QEMU monitor is accessible by pressing `Ctrl+A C` when running with `-serial stdio`.
It provides direct inspection of the emulated machine state without stopping the guest CPU:

| Command             | Description                                                    |
|---------------------|----------------------------------------------------------------|
| `info registers`    | Dump all CPU registers                                         |
| `info mem`          | Show active virtual memory mappings                            |
| `xp /16xw 0x100000` | Read 16 words from physical address 0x100000 (bypasses paging) |
| `info mtree`        | Print the full memory region tree                              |
| `info irq`          | Show IRQ statistics                                            |
| `quit`              | Terminate QEMU immediately                                     |
