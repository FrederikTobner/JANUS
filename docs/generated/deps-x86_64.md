> *Generated for `x86_64` — run `cmake --preset <x86_64-preset>` to regenerate.*

```mermaid
graph TD
  subgraph lib_layer["Library Layer"]
    display("display")
    fmt("fmt")
    page_tables("page_tables")
  end
  subgraph core_layer["Core Layer"]
    kio(["kio"])
  end
  subgraph subsys_layer["Subsystem Layer"]
    boot["boot"]
    drivers["drivers"]
    interrupts["interrupts"]
    mm["mm"]
    kmain["kmain"]
  end
  subgraph proto_layer["Protocol Libraries"]
    boot_limine[["boot_limine"]]
    boot_multiboot2[["boot_multiboot2"]]
  end
  subgraph exec_layer["Executables"]
    kernel_limine_elf{{"kernel-limine.elf"}}
    kernel_multiboot2_elf{{"kernel-multiboot2.elf"}}
  end
  subgraph asm_layer["ASM Layer"]
    janus_asm[("janus_asm")]
  end
  subgraph contract_layer["Contracts"]
    janus_contract_memmap(("memmap"))
  end
  page_tables --> janus_asm
  kio --> fmt
  kio --> janus_asm
  boot --> display
  boot -.-> janus_contract_memmap
  boot_limine --> boot
  boot_multiboot2 --> boot
  drivers --> display
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
  kernel_multiboot2_elf --> boot
  kernel_multiboot2_elf --> boot_multiboot2
  kernel_multiboot2_elf --> drivers
  kernel_multiboot2_elf --> interrupts
  kernel_multiboot2_elf --> mm
  kernel_multiboot2_elf --> kmain
```
