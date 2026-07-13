# JANUS Kernel Main Module

Core kernel initialization and orchestration.

This is the **only** module allowed to depend on subsystems.
It sits between the assembly entry points (`_start`) and the subsystem layer (`subsys/`).
