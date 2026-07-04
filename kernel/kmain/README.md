# JANUS Kernel Main Module

Core kernel initialization and orchestration.

This is the **only** module allowed to depend on subsystems.
It sits between the assembly entry points (`_start`) and the subsystem layer (`subsys/`).

## Output Sink

`output_sink.c` fans `kio` character output to whichever drivers are active (serial, console, or both).
It is the sole caller of `kio_register_putc` and owns the `g_serial_active` / `g_console_active` flags.
