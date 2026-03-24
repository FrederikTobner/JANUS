# Debugging a Kernel with LLDB

This tutorial walks through a complete LLDB debugging session against a kernel running in QEMU. The goal is to verify — not assume — that the boot process worked: the Multiboot2 magic is correct, the info pointer is valid, and the CPU is executing our C code. The same workflow applies any time you need to inspect registers, step through assembly, or verify memory layout.

## Prerequisites

LLDB and QEMU must be installed (see [Dependencies](../project/setup/dependencies.md)). Familiarity with the Multiboot2 entry state helps — see [Multiboot2](../concepts/boot/multiboot2.md). For a quick command reference, see [LLDB](../concepts/tools/lldb.md).

## The Setup

QEMU has a built-in GDB stub. When we start QEMU with `-s -S`, it:

- Opens a debug port on `localhost:1234` (`-s`)
- Pauses the CPU at the first instruction (`-S`)

LLDB connects to this port and gains full control: breakpoints, single-stepping, register and memory inspection.

```
┌──────────────┐        TCP :1234        ┌────────────────┐
│  Terminal 1   │ ◄────────────────────► │     QEMU       │
│  (LLDB)       │   GDB Remote Protocol  │  (CPU paused)  │
└──────────────┘                         └────────────────┘
```

## Starting the Debug Session

**Terminal 1** — launch QEMU in debug mode:

```bash
qemu-system-x86_64 -cdrom build/janus.iso -boot d -serial stdio -s -S
```

QEMU opens but the virtual CPU is frozen. No code executes until the debugger says so.

**Terminal 2** — connect LLDB:

```bash
lldb build/kernel.elf
```

At the `(lldb)` prompt:

```
(lldb) gdb-remote localhost:1234
```

LLDB loads our kernel's symbol table and attaches to QEMU's virtual CPU. At this point the instruction pointer is at the BIOS reset vector (`0xFFF0`), before any of our code has run.

## Setting a Breakpoint

Tell LLDB to pause when execution reaches `kernel_main`:

```
(lldb) b kernel_main
Breakpoint 1: where = kernel.elf`kernel_main + 15 at main.c:42
```

Now let the kernel boot. GRUB will load the kernel, jump to `_start`, the boot assembly switches to long mode, and eventually calls `kernel_main` — where the breakpoint fires:

```
(lldb) c
Process 1 resuming
Process 1 stopped
* thread #1, stop reason = breakpoint 1.1
    frame #0: kernel.elf`kernel_main(magic=920085129, info=0x00000000001010e0) at main.c:42:15
```

## Inspecting Boot State

### Verifying the Multiboot2 Magic

The first argument to `kernel_main` is the magic number. Check it:

```
(lldb) p/x magic
(uint32_t) $0 = 0x36d76289
```

`0x36D76289` is the Multiboot2 bootloader magic — GRUB passed the correct value. If this were wrong, the bootloader either did not use Multiboot2 or the ABI convention was violated.

### Verifying the Info Pointer

```
(lldb) p/x info
(void *) $1 = 0x00000000001010e0
```

A non-null address in the low megabytes is expected. This points to the Multiboot2 information structure that GRUB populated with memory maps, ELF section headers, and similar metadata.

### Register Dump

For a full view of the CPU state:

```
(lldb) register read
general:
       rax = 0x0000000080000000
       rbx = 0x00000000001010c0
       rdi = 0x0000000036d76289      ← magic (1st argument)
       rsi = 0x00000000001010e0      ← info  (2nd argument)
       rbp = 0x0000000000106ff0      ← frame pointer (on stack)
       rsp = 0x0000000000106fe0      ← stack pointer
       rip = 0x000000000010109f      ← instruction pointer
```

`rdi` and `rsi` confirm the System V AMD64 calling convention: the first two integer/pointer arguments arrive in these registers.

## Stepping Through Code

Step to the next source line with `n` (step over) — this executes the `if (magic != ...)` check:

```
(lldb) n
    frame #0: kernel.elf`kernel_main at main.c:51:15
-> 51       if (info == NULL) {
```

The magic check passed. Step again:

```
(lldb) n
    frame #0: kernel.elf`kernel_main at main.c:60:12
-> 60       for (;;) {
```

The null check passed too. The kernel is now sitting in the infinite halt loop — exactly as designed.

## Useful Commands Reference

| Command | Purpose |
| ------- | ------- |
| `gdb-remote localhost:1234` | Attach to QEMU |
| `b kernel_main` | Break at function entry |
| `b main.c:42` | Break at a specific line |
| `c` | Continue until next breakpoint |
| `n` | Step over (next source line) |
| `s` | Step into (enter function calls) |
| `stepi` | Execute one machine instruction |
| `p/x $rdi` | Print register in hex |
| `frame variable` | Show local variables |
| `register read` | Dump all general-purpose registers |
| `bt` | Backtrace (show call stack) |
| `memory read 0xB8000 -c 16` | Read 16 bytes of memory |

## Troubleshooting

**LLDB says "Failed to connect"** — QEMU must be running with `-s -S`. Verify the process is alive and port 1234 is open (`ss -tlnp | grep 1234`).

**"Unable to resolve breakpoint"** — the kernel was likely built without debug symbols. Verify with `file build/kernel.elf` — the output should mention "not stripped" and "with debug_info". Ensure `-g` is in the compiler flags.

**Breakpoint fires at an unexpected address** — if the kernel uses higher-half mappings, LLDB may resolve the symbol to a virtual address that does not match the physical address where the code actually executes early in boot. Use `b -a 0x10109f` (or whatever `readelf -s` reports) to set a breakpoint at a physical address directly.

## Key Takeaways

- **Never assume the boot worked** — verify with a debugger. A black screen can mean success or a triple fault.
- **The GDB stub is protocol-compatible** — LLDB, GDB, and other tools that speak the GDB Remote Serial Protocol all work with QEMU's `-s` flag.
- **Register inspection beats `printf` in early boot** — before any I/O driver exists, the debugger is the only window into the machine.
