# Wiring Kernel Output and Panic

This tutorial shows how to route kernel text output through JANUS' shared `kio` service and how to use `kpanic()` consistently from subsystem code.

## Goal

By the end you should have:

- a console backend callback registered with `kio_register_putc()`
- `kprintf()` usable from subsystems without direct driver coupling
- `kpanic()` available for fail-fast invariants

## 1. Register the Output Callback

`kio` writes characters through a callback. Register it after console backends are initialized:

```c
static void console_putc(char c)
{
    if (g_serial_active) {
        drivers_serial_putc(c);
    }
    if (g_tty_active) {
        drivers_tty_putc(c);
    }
}

void console_init_early(void)
{
    if (drivers_serial_init(...) == JANUS_OK) {
        g_serial_active = true;
        kio_register_putc(console_putc);
    }
}
```

You can re-register after full console setup (for example after TTY initialization) to upgrade the sink.

## 2. Print from Subsystems via kprintf

Subsystem code includes `<kio/kio.h>` and prints through `kprintf`:

```c
#include <kio/kio.h>

kprintf("pmm: free pages = %llu\n", (unsigned long long) stats.free_pages);
```

No direct dependency on serial or TTY driver internals is needed.

## 3. Use kpanic for Invariant Violations

Use `kpanic()` for unrecoverable kernel errors:

```c
if (UNLIKELY(!g_pmm.initialized)) {
    kpanic("mm_pmm_alloc_page: PMM not initialized");
}
```

`kpanic` captures `__FILE__` and `__LINE__` automatically and halts after printing.

## 4. Keep Architecture Logic out of Call Sites

`kpanic` halting is implemented behind `kio` architecture backends selected by CMake. Callers should never branch on `__x86_64__`/`__aarch64__` for panic behavior.

## Troubleshooting

- If `kprintf` prints nothing early: verify `kio_register_putc()` was called.
- If serial works but TTY does not: confirm callback re-registration after full console init.
- If panic message is missing but system halts: callback likely not registered yet, but halt path is still functioning.

## Related Reading

- [Kernel Output and Panic Flow](../../wiki/io/kernel-output-panic.md)
- [Core kio Reference](../../project/subsystems/core/kio.md)
- [Writing a Serial Port Driver](serial-port-driver.md)
