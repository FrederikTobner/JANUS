# Kernel Smoke Tests

Black-box "does it boot and reach known-good milestones?" tests. They build the
real ISO, boot it in QEMU headless, and assert on the serial console output. See
the design and implementation specs under
[`docs/specs/smoke-tests/`](../../../docs/specs/smoke-tests/).

These are **host-side** tests (a Lua runner + CTest glue), not a kernel module:
nothing here is compiled into a kernel artifact, so they never enter the kernel's
layered dependency graph even though they live under `kernel/`. The repository-root
`tests/` tree is reserved for future user-space application test suites.

## Layout

| File             | Responsibility                                                      |
|------------------|---------------------------------------------------------------------|
| `run_smoke.lua`  | Launch QEMU under `timeout(1)`, scrape serial, assert, report.      |
| `profiles.lua`   | The marker tables (`nominal`, `fault`) — the single source of truth. |

## Requirements

- A Lua interpreter (`lua` 5.3/5.4 or `luajit`).
- GNU coreutils (`timeout`, `kill`) — standard on Linux.
- QEMU for the target arch, and for aarch64 the UEFI firmware (see
  [`docs/01-getting-started.md`](../../../docs/01-getting-started.md)).

## Running via CTest (recommended)

Smoke tests are off by default; enable them at configure time:

```bash
# Nominal profile (clean boot) across every boot protocol for the preset:
cmake --preset x86_64-gcc -DJANUS_ENABLE_SMOKE_TESTS=ON
ctest --preset x86_64-gcc -R smoke

# Fault profile (deliberate fault must be trapped and panic cleanly).
# JANUS_TEST_FAULTS is a configure-time switch, so use a separate build dir:
cmake -S . -B build-x86_64-gcc-fault -G Ninja \
      --toolchain cmake/toolchains/x86_64-gcc.cmake \
      -D JANUS_TARGET_ARCH=x86_64 \
      -D JANUS_ENABLE_SMOKE_TESTS=ON -D JANUS_TEST_FAULTS=ON
ctest --test-dir build-x86_64-gcc-fault -R smoke --output-on-failure
```

The ISO is built automatically by a CTest setup fixture before each run. If QEMU
(or aarch64 firmware) is missing, the test reports as **skipped**, not failed.

## Running the runner directly

Useful when iterating on the harness itself:

```bash
lua kernel/tests/smoke/run_smoke.lua \
    --qemu qemu-system-x86_64 \
    --iso  build-x86_64-gcc/janus_x86_64.iso \
    --profile nominal
```

Exit codes: `0` pass, `1` fail, `2` usage error, `77` skipped.

## Adding or changing a marker

Edit `profiles.lua` only — it is the one place console strings live. Use `substr`
for a literal substring or `num_gt` for a numeric-threshold check. Keep markers few
and stable (prefer short tokens like `Version:` over full lines).
