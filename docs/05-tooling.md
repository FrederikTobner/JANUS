# Tooling

## CMake

CMake 3.20+ with Ninja as backend. Presets bundle the generator, toolchain, and
build directory:

```bash
cmake --preset x86_64-gcc          # configure
cmake --build --preset x86_64-gcc  # build
cmake --build --preset x86_64-gcc --target run
```

### Useful Commands

```bash
# Configure and build
cmake -B build -G Ninja
cmake --build build
cmake --build build --target iso

# Inspect cache
cmake -B build -G Ninja -LH          # list variables with help strings
cat build/CMakeCache.txt | grep JANUS

# Clean rebuild
cmake --build build --target clean
rm -rf build && cmake -B build -G Ninja

# Debug CMake
cmake -B build -G Ninja --trace-expand   # full execution trace
cmake --build build --verbose            # raw compiler commands
```

`compile_commands.json` is written to the build directory and copied to the project
root for clangd. If IDE autocomplete is broken, confirm this file is current.

## QEMU

The `run` build target invokes QEMU with the correct flags. Underlying commands:

```bash
# x86_64 Limine
qemu-system-x86_64 -cdrom build/janus.iso -serial stdio -m 256M

# aarch64 Limine
qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel build/kernel.elf -serial stdio -m 256M
```

### Useful Flags

| Flag | Effect |
|---|---|
| `-serial stdio` | Redirect virtual serial port to terminal |
| `-m 256M` | Set guest RAM |
| `-no-reboot` | Exit on triple fault instead of rebooting |
| `-no-shutdown` | Keep QEMU open after guest halts |
| `-d int,cpu_reset` | Log interrupts and CPU resets to stderr |
| `-D qemu.log` | Write debug output to file |
| `-s` | GDB server on `localhost:1234` |
| `-S` | Freeze CPU at startup (wait for debugger) |

### QEMU Monitor

Access with `Ctrl+A C` when running with `-serial stdio`:

```
(qemu) info registers
(qemu) info mem           # active page table mappings
(qemu) xp /16xw 0x100000  # read physical memory (bypasses paging)
(qemu) info mtree          # memory region tree
```

### Common Pitfalls

- **No serial output:** confirm `-serial stdio` is present.
- **Triple fault on boot:** use `-no-reboot -d int,cpu_reset` to catch it.
- **aarch64 requires `-M virt`:** no default machine type; must be specified.
- **UEFI mode:** pass `-bios /usr/share/OVMF/OVMF_CODE.fd`; the `run-uefi` target handles this.

## LLDB

LLDB is the primary debugger. The repository's `.lldbinit` automates connection.

```bash
# Start QEMU with debug server
cmake --build --preset x86_64-gcc --target debug-limine
# Connect from LLDB
(lldb) gdb-remote localhost:1234
```

### Essential Commands

```
# Execution
c         process continue
n         thread step-over
s         thread step-in
si        thread step-inst

# Breakpoints
b kernel_main
b main.c:42
breakpoint set --address 0x100000

# Inspect
register read
p my_variable
p/x my_variable
p *(boot_context_t *)ctx
frame variable
bt

# Memory
memory read 0xB8000 --count 64
x/16xw 0xB8000

# Disassembly
disassemble --frame
disassemble --name boot_init
```

**Notes:** symbol names are your own (no libc, no `main` — look for `kernel_main`
or `_start`). Single-stepping through I/O code changes device state for real.
Watchpoints on MMIO addresses may not behave as expected.

## Doxygen

```bash
doxygen Doxyfile
```

HTML output is written to `doxygen-out/html/`. The Doxygen API reference is
published to GitHub Pages at `/JANUS/api/` on every push to `main`.

Writing good API docs:

- Use `@file` and `@brief` at file scope.
- Document public structs and their invariants.
- Keep parameter and return semantics explicit.

## clang-tidy

Static analysis gate. Runs in CI on the `x86_64-clang` build cell.

```bash
# Configure first (needs compile_commands.json)
cmake --preset x86_64-clang -D CMAKE_BUILD_TYPE=Release
cmake --build --preset x86_64-clang

# Run on all kernel C files
jq -r '.[].file' build-x86_64-clang/compile_commands.json \
  | grep 'kernel/.*\.c$' \
  | xargs clang-tidy -p build-x86_64-clang

# Single file
clang-tidy -p build-x86_64-clang kernel/subsys/mm/src/pmm.c
```

Active checks are configured in `.clang-tidy` at the repository root. Fix root
causes rather than suppressing checks where possible.
