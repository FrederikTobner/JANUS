# Dependency Graph

This page visualises the relationships between all registered targets.
Separate diagrams are provided for the kernel and for the tools:

- **Kernel** — all subsystems, libraries, protocol libraries, and executables
- **Tools** — all tool libraries and tool executables

---

## Kernel Targets

Two views are provided for the kernel:

- **Module dependencies** — which targets depend on which other targets (generated from CMake at configure time)
- **Build artifacts** — which object files and protocol libraries are combined into each executable

## Module Dependencies

{{#include ../../generated/deps.md}}

> Node shapes: rounded rectangle = library · rectangle = subsystem · double rectangle = protocol library · hexagon = executable

## Build Artifacts

The kernel is linked into separate ELF binaries, one per boot protocol.
Assembly and C object files are combined with the subsystem libraries by the linker.

```mermaid
graph LR
  subgraph x86["x86_64"]
    direction TB
    subgraph limine_x86["kernel-limine.elf"]
      direction TB
      x_entry_common["_start_common\ngdt64.asm"]
      x_entry_limine["_start_limine\nentry.asm"]
      x_limine_req["_start_limine_requests\nlimine_requests.c"]
    end
    subgraph mb2["kernel-multiboot2.elf"]
      direction TB
      x_entry_common2["_start_common\ngdt64.asm"]
      x_entry_mb2["_start_multiboot2\nentry.asm · header.asm · paging.asm"]
    end
  end
  subgraph aarch64["aarch64"]
    direction TB
    subgraph limine_aa64["kernel-limine.elf"]
      direction TB
      aa_entry_limine["_start_limine\nentry.S"]
      aa_limine_req["_start_limine_requests\nlimine_requests.c"]
    end
  end

  kmain(["kmain"])
  boot_limine[["boot_limine"]]
  boot_multiboot2[["boot_multiboot2"]]
  subsystems(["boot · drivers · fmt · …"])

  x_entry_common & x_entry_limine & x_limine_req & kmain & boot_limine & subsystems --> limine_x86
  x_entry_common2 & x_entry_mb2 & kmain & boot_multiboot2 & subsystems --> mb2
  aa_entry_limine & aa_limine_req & kmain & boot_limine & subsystems --> limine_aa64
```

---

## Tool Targets

{{#include ../../generated/deps-tools.md}}

> Node shapes: rounded rectangle = tool library · hexagon = tool executable
