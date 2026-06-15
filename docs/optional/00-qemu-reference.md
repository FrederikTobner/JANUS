# QEMU Reference

## Invocation

The `run` and `debug-*` CMake targets invoke QEMU with the correct flags for the
selected preset. Direct invocation is rarely necessary, but the underlying commands
are:

```bash
# x86_64 Limine ISO
qemu-system-x86_64 -cdrom build-x86_64-gcc/janus.iso -serial stdio -m 256M

# aarch64 Limine
qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel build-aarch64-gcc/kernel.elf \
    -serial stdio -m 256M
```

## Useful Flags

| Flag | Effect |
|---|---|
| `-serial stdio` | Redirect the virtual serial port to the host terminal |
| `-m 256M` | Set guest RAM size |
| `-no-reboot` | Exit the VM instead of rebooting on triple fault |
| `-no-shutdown` | Keep QEMU open after the guest halts |
| `-d int,cpu_reset` | Log all interrupts and CPU resets to stderr |
| `-D qemu.log` | Write debug log to a file instead of stderr |
| `-s` | Start a GDB server on `localhost:1234` |
| `-S` | Freeze the CPU at startup and wait for a debugger connection |
| `-bios /usr/share/OVMF/OVMF_CODE.fd` | Boot in UEFI mode (x86_64) |

## QEMU Monitor

The QEMU monitor is accessible by pressing `Ctrl+A C` when running with
`-serial stdio`. It provides direct inspection of the emulated machine state
without stopping the guest CPU:

| Command | Description |
|---|---|
| `info registers` | Dump all CPU registers |
| `info mem` | Show active virtual memory mappings |
| `xp /16xw 0x100000` | Read 16 words from physical address 0x100000 (bypasses paging) |
| `info mtree` | Print the full memory region tree |
| `info irq` | Show IRQ statistics |
| `quit` | Terminate QEMU immediately |

## Common Pitfalls

- **No serial output:** confirm `-serial stdio` is present. If the output target
  is a file, use `-serial file:serial.log` and `tail -f serial.log` in a separate
  terminal.
- **Triple fault on boot:** use `-no-reboot -d int,cpu_reset` together. QEMU exits
  with a non-zero code on triple fault, making it easy to detect in scripts.
- **aarch64 requires `-M virt`:** there is no default machine type for aarch64;
  omitting it is an error.
- **UEFI mode on x86_64:** pass `-bios /usr/share/OVMF/OVMF_CODE.fd`; the
  `run-uefi` build target already includes this flag.
- **Clock drift under heavy load:** `-icount shift=auto` enables a deterministic
  instruction counter that improves reproducibility when debugging timing-sensitive
  code.
