# Summary

[Introduction](introduction.md)

---

# Project

- [Overview](project/introduction.md)

---

- [Setup](project/setup/setup.md)
  - [Dependencies](project/setup/dependencies.md)

---

- [Architecture](project/architecture/architecture.md)
  - [Layer Model](project/architecture/layer-model.md)
  - [Module Structure](project/architecture/module-structure.md)
  - [Include Hierarchy](project/architecture/include-hierarchy.md)
  - [Privilege Model](project/architecture/privilege-model.md)
  - [Dependency Graph](project/architecture/dependency-graph.md)

---

- [Subsystems](project/subsystems/subsystems.md)
  - [Boot](project/subsystems/boot/README.md)
    - [Boot Context](project/subsystems/boot/context.md)
    - [Limine Protocol](project/subsystems/boot/limine.md)
    - [Multiboot2 Protocol](project/subsystems/boot/multiboot2.md)
  - [Kernel Main](project/subsystems/kmain/README.md)
    - [kernel_main](project/subsystems/kmain/main.md)
    - [Init Sequence](project/subsystems/kmain/init.md)
    - [Kernel Descriptor](project/subsystems/kmain/kernel-descriptor.md)
  - [Drivers](project/subsystems/drivers/README.md)
    - [TTY](project/subsystems/drivers/tty.md)
    - [Serial](project/subsystems/drivers/serial.md)
    - [CPU](project/subsystems/drivers/cpu.md)

---

- [Tooling](project/tooling/tooling.md)
  - [LLDB](project/tooling/lldb.md)
  - [CMake](project/tooling/cmake.md)
  - [QEMU](project/tooling/qemu.md)

---

- [Tools](project/tools/tools.md)
  - [Philosophy](project/tools/philosophy.md)
  - [Standards](project/tools/standards.md)
  - [Catalog](project/tools/catalog.md)
    - [clap](project/tools/clap.md)
    - [psf2c](project/tools/psf2c.md)
  - [Dependency Graph](project/tools/dependency-graph.md)

---

- [Contributing](project/contributing/contributing.md)
  - [Coding Style](project/contributing/coding-style.md)
  - [CI Workflows](project/contributing/ci.md)
  - [Terminology](project/contributing/terminology.md)

---

- [Principles](project/principles/principles.md)
  - [Philosophy](project/principles/philosophy.md)
  - [Standards](project/principles/standards.md)

---

# Wiki

- [Overview](wiki/introduction.md)

---

- [Boot](wiki/boot/boot.md)
  - [Firmware](wiki/boot/firmware.md)
  - [Bootloaders](wiki/boot/bootloaders.md)
  - [Freestanding Environments](wiki/boot/freestanding.md)
  - [Protocols](wiki/boot/protocols/protocols.md)
    - [Limine](wiki/boot/protocols/limine.md)
    - [Multiboot2](wiki/boot/protocols/multiboot2.md)

---

- [Memory](wiki/memory/memory.md)
  - [Physical Memory](wiki/memory/physical-memory.md)
  - [Virtual Memory](wiki/memory/virtual-memory.md)

---

- [I/O](wiki/io/io.md)
  - [Serial Ports](wiki/io/serial-ports.md)
  - [Memory-Mapped I/O](wiki/io/mmio.md)

---

- [Graphics](wiki/graphics/graphics.md)
  - [VGA Text Mode](wiki/graphics/vga-text-mode.md)
  - [Framebuffers](wiki/graphics/framebuffers.md)

---

# Tutorials

- [Overview](tutorials/introduction.md)

---

- [Boot](tutorials/boot/boot.md)
  - [Writing a Kernel Linker Script](tutorials/boot/linker-scripts.md)
  - [Transitioning to 64-bit Long Mode](tutorials/boot/x86-64-mode-switch.md)
  - [Creating a Bootable ISO](tutorials/boot/bootable-iso.md)

---

- [I/O](tutorials/io/io.md)
  - [Writing a Serial Port Driver](tutorials/io/serial-port-driver.md)

---

- [Graphics](tutorials/graphics/graphics.md)
  - [Writing a VGA Text Buffer Driver](tutorials/graphics/vga-text-buffer.md)

---

- [Debugging](tutorials/debugging/debugging.md)
  - [Debugging a Kernel with LLDB](tutorials/debugging/kernel-debugging-lldb.md)
