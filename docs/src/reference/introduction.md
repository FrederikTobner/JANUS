# JANUS Reference

JANUS (Just ANother Unix-like System) is an educational operating system kernel written in C17 and assembly, targeting x86_64 and aarch64. This reference documents how the kernel is structured, how its subsystems interact, and the reasoning behind key design decisions.

The reference assumes familiarity with the general concepts involved in OS development. Background material on topics like virtual memory, boot protocols, and framebuffers is available in the [Concepts](../concepts/introduction.md) section — this part focuses on the JANUS-specific implementation.

Because JANUS already ships Doxygen-annotated source code, this reference deliberately stays at the architectural level. It describes the high-level design, the module boundaries, and the data flow between subsystems, rather than duplicating the function-level documentation that lives in the code itself.

The reference is organised into the following sections:

- **Architecture** covers the layer model, the module structure, and the three-tier include hierarchy that governs how subsystems expose their APIs.
- **Boot** describes the boot subsystem — the `boot_context_t` structure, Limine and Multiboot2 protocol handling, and the data flow from firmware to `kmain`.
- **Kernel Main** explains the `kmain` layer: the `kernel_descriptor_t`, the initialisation sequence, and the rules about which modules may depend on what.
- **Drivers** documents the device driver subsystem — serial output, TTY rendering, and CPU control.
