# kmalloc

Core-layer kernel heap allocator service.

## Responsibilities

- Provide `kmalloc`, `kcalloc`, `krealloc`, `kfree` and stats reporting.
- Manage slab caches and slab lifecycle.
- Stay decoupled from subsystems by consuming registered page-source callbacks.

## Dependency Rules

- May depend on `lib/` modules (`mem`).
- Must not depend directly on subsystem modules.
- Physical page allocation is injected by `kmain` via registration API.

## Verification

Build with `-DJANUS_TEST_KMALLOC=ON` to run the kmalloc self-test from
`kmain` right after allocator initialization:

```bash
cmake --preset x86_64-gcc -DJANUS_TEST_KMALLOC=ON
cmake --build --preset x86_64-gcc
qemu-system-x86_64 -cdrom build-x86_64-gcc/janus_x86_64.iso -serial stdio
```

A successful run prints `[kmalloc-test] all 11 cases passed`. Any failing
case panics with a case-specific message.
