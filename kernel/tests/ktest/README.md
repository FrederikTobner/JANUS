# Kernel KTest

This directory contains the optional in-kernel runtime test framework and the host-side runner used to validate it under QEMU.

## Layout

| File | Responsibility |
| --- | --- |
| `include/ktest/ktest.h` | Public `KTEST()` and assertion API used by kernel modules. |
| `src/ktest.c` | Registry walker and summary reporter. |
| `src/ktest_exit.c` | QEMU exit handling for pass/fail exit codes. |
| `run_ktest.lua` | Host-side Lua runner that boots the built ISO and asserts on the runtime ktest output. |

## Running directly

```bash
lua kernel/tests/ktest/run_ktest.lua \
    --qemu qemu-system-x86_64 \
    --iso build-x86_64-gcc/janus_x86_64.iso \
    --timeout 60
```

The runner exits with `0` on success, `1` on a failing kernel test, `2` on usage errors, and `77` when QEMU or firmware is unavailable.

## Enabling the suite

```bash
cmake --preset x86_64-gcc -D JANUS_ENABLE_KTEST=ON
ctest --preset x86_64-gcc -R ktest --output-on-failure
```

The host-side runner is deliberately kept separate from the smoke harness. Smoke tests remain focused on boot-level serial validation, while this ktest runner validates the runtime KTest registry itself.
