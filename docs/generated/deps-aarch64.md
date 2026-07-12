> *Generated for `aarch64` — run `cmake --preset <aarch64-preset>` to regenerate.*

```mermaid
graph TD
  subgraph library_layer["Library Layer"]
    gfx("gfx")
    fmt("fmt")
    page_tables("page_tables")
  end
  subgraph core_layer["Core Layer"]
    kio(["kio"])
  end
  subgraph subsystem_layer["Subsystem Layer"]
    boot["boot"]
    drivers["drivers"]
    interrupts["interrupts"]
    mm["mm"]
    kmain["kmain"]
  end
  subgraph protocol_libraries["Protocol Libraries"]
    boot_limine[["boot_limine"]]
  end
  subgraph executables["Executables"]
    kernel_limine_elf{{"kernel-limine.elf"}}
  end
  subgraph asm_layer["ASM Layer"]
    janus_asm[("janus_asm")]
  end
  subgraph contracts["Contracts"]
    janus_contract_memmap(("memmap"))
    janus_contract_display(("display"))
  end
  page_tables --> janus_asm
  kio --> fmt
  kio --> janus_asm
  boot -.-> janus_contract_memmap
  boot -.-> janus_contract_display
  boot_limine --> boot
  drivers -.-> janus_contract_display
  interrupts --> kio
  mm --> kio
  mm -.-> janus_contract_memmap
  kmain --> drivers
  kmain --> boot
  kmain --> fmt
  kmain --> interrupts
  kmain --> kio
  kmain --> mm
  kernel_limine_elf --> boot
  kernel_limine_elf --> boot_limine
  kernel_limine_elf --> drivers
  kernel_limine_elf --> interrupts
  kernel_limine_elf --> mm
  kernel_limine_elf --> kmain
```
