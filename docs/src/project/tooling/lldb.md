# LLDB

LLDB is the debugger from the LLVM project. It serves the same role as GDB but uses a more consistent, self-documenting command syntax. JANUS uses LLDB as the primary debugger.

## Connecting to QEMU

QEMU exposes a GDB-compatible debug server when started with `-s -S` (or the `debug` build target). LLDB connects to it over TCP:

```
(lldb) gdb-remote localhost:1234
```

The repository's `.lldbinit` file automates this — it sets the target architecture, loads the kernel ELF for symbols, and connects.

## Essential Commands

### Execution Control

| Command | Short | Effect |
|---------|-------|--------|
| `process continue` | `c` | Resume execution |
| `thread step-over` | `n` | Step over (next line) |
| `thread step-in` | `s` | Step into a function call |
| `thread step-out` | `finish` | Run until the current function returns |
| `thread step-inst` | `si` | Step one instruction |
| `process interrupt` | | Break into the debugger (Ctrl+C also works) |

### Breakpoints

```
(lldb) breakpoint set --name kernel_main         # Break on function
(lldb) b kernel_main                              # Short form
(lldb) breakpoint set --file main.c --line 42     # Break on file:line
(lldb) b main.c:42                                # Short form
(lldb) breakpoint set --address 0x100000          # Break on raw address
(lldb) breakpoint list                            # Show all breakpoints
(lldb) breakpoint delete 1                        # Delete breakpoint #1
```

### Inspecting State

```
(lldb) register read                              # All general-purpose registers
(lldb) register read rsp rbp rip                  # Specific registers
(lldb) p my_variable                              # Print a variable
(lldb) p/x my_variable                            # Print in hexadecimal
(lldb) p *(boot_context_t *)ctx                   # Dereference and print a struct
(lldb) frame variable                             # All locals in current frame
(lldb) bt                                         # Backtrace
```

### Memory Examination

```
(lldb) memory read 0xB8000 --count 64             # Read 64 bytes at address
(lldb) x/16xw 0xB8000                             # 16 words in hex (GDB-style)
(lldb) memory read --size 4 --format x 0xB8000    # 4-byte units, hex
```

### Disassembly

```
(lldb) disassemble --frame                        # Current function
(lldb) disassemble --name boot_init               # Named function
(lldb) disassemble --start-address 0x100000 --count 20  # Raw address range
```

## Kernel Debugging Tips

**No standard library symbols.** In a freestanding kernel, there is no libc. Symbol names are your own, and you will not see `main` — look for `kernel_main` or `_start`.

**Physical vs. virtual addresses.** If paging is enabled (Limine), LLDB sees virtual addresses. Breakpoints on physical addresses will not hit. Use the virtual addresses from the ELF symbol table.

**Volatile hardware state.** Single-stepping through I/O code (serial, VGA) changes device state. Stepping over `outb()` to COM1 sends a character for real. Watchpoints on MMIO addresses may not behave as expected.

**QEMU monitor.** QEMU's monitor (`Ctrl+A C` in the terminal) provides `info registers`, `info mem` (page table dump), and `xp` (physical memory read) — useful when LLDB cannot see what you need.

## Troubleshooting

**"Failed to connect to localhost:1234".** QEMU must be running in debug mode (`ninja -C build debug` or the `-s -S` flags). Verify that the debug target is actually waiting for a connection before launching LLDB.

**"Unable to resolve breakpoint to any actual locations".** The kernel binary may not contain debug symbols. Verify with `file build/kernel.elf` — the output should indicate the file is not stripped. If symbols are missing, check that the build configuration includes `-g`.

**Breakpoint hits at the wrong address.** If the kernel uses a higher-half mapping, the symbol addresses in the ELF differ from the physical load address. LLDB uses the ELF's virtual addresses, which is correct after paging is active. For early boot code before paging, set breakpoints by raw address (`b -a 0x100000`).
