# QEMU

QEMU is a machine emulator that can boot a kernel image without real hardware. It is the standard development and testing tool for OS kernels — fast to iterate with, supports both x86_64 and aarch64, and provides debug facilities that bare metal does not.

## Running a Kernel

The `run` build target invokes QEMU with the correct flags for the current architecture and boot protocol. Under the hood, the command looks roughly like:

```bash
# x86_64 with Limine ISO
qemu-system-x86_64 -cdrom build/janus.iso -serial stdio -m 128M

# aarch64 with Limine
qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel build/kernel.elf -serial stdio -m 128M
```

## Essential Flags

| Flag | Effect |
|------|--------|
| `-serial stdio` | Redirect the virtual serial port to the host terminal. |
| `-serial file:serial.log` | Write serial output to a file instead. |
| `-m 128M` | Set guest RAM to 128 MiB. |
| `-no-reboot` | Exit on triple fault instead of rebooting. Essential for catching fatal errors. |
| `-no-shutdown` | Keep QEMU open after the guest halts. |
| `-d int,cpu_reset` | Log interrupts and CPU resets to stderr. |
| `-D qemu.log` | Write QEMU debug output to a file. |
| `-s` | Start a GDB server on `localhost:1234`. |
| `-S` | Freeze the CPU at startup (wait for debugger attach). |

## Debugging with QEMU

Combining `-s -S` starts QEMU with the CPU halted and a debug server listening. Connect from LLDB or GDB:

```
(lldb) gdb-remote localhost:1234
(gdb) target remote localhost:1234
```

The QEMU monitor provides additional introspection. Access it with `Ctrl+A C` when running with `-serial stdio`:

```
(qemu) info registers             # Dump CPU registers
(qemu) info mem                   # Show active page table mappings
(qemu) xp /16xw 0x100000         # Read physical memory (bypasses paging)
(qemu) info mtree                 # Memory region tree (MMIO layout)
```

## Common Pitfalls

**Serial output not appearing.** Check that `-serial stdio` (or `-serial file:...`) is present. Without it, the virtual UART exists but its output goes nowhere.

**Triple fault on boot.** Use `-no-reboot -d int,cpu_reset` to catch the fault instead of endlessly rebooting. The fault log usually identifies the offending address.

**UEFI mode.** To test UEFI boot, pass `-bios /usr/share/OVMF/OVMF_CODE.fd` (path varies by distribution). The `run-uefi` build target handles this.

**aarch64 requires `-M virt`.** Unlike x86_64, QEMU's aarch64 has no default machine type. You must specify `-M virt` (or another machine) explicitly.

**QEMU window not visible.** On tiling window managers (i3, sway), QEMU may default to VNC output. Force a visible window with `-display sdl` or `-display gtk`. Alternatively, use a VNC viewer.

**Kernel appears to hang on boot.** A blank screen after "Booting from DVD/CD..." is normal if the kernel has no video or serial output yet. Use the `debug` build target and connect LLDB to verify execution — see [LLDB](lldb.md).

