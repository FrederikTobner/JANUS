# mm — Memory Management Subsystem

The `mm` subsystem owns physical memory allocation and memory-management lifecycle.

## Responsibilities

- **Physical memory manager (PMM)**
- **Virtual memory manager (VMM)**
- **TLB management**

## Verification

Build with `-DJANUS_TEST_KMALLOC=ON` to run the kmalloc heap allocator
self-test from `kmain` right after the allocator initializes, and inspect the
results over serial in QEMU:

```bash
cmake --preset x86_64-gcc -DJANUS_TEST_KMALLOC=ON
cmake --build --preset x86_64-gcc
qemu-system-x86_64 -cdrom build-x86_64-gcc/janus_x86_64.iso -serial stdio
```

A successful run prints `[kmalloc-test] all 11 cases passed` and the kernel
continues booting to a normal halt; any check failure `kpanic`s with a
message identifying the specific failing case. `JANUS_TEST_KMALLOC` is
mutually exclusive with `JANUS_TEST_FAULTS` (see
[kernel/subsys/interrupts/README.md](../interrupts/README.md)) — both redirect
`kmain`, and CMake will refuse to configure with both enabled.
