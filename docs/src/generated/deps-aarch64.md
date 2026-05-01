> *Generated for `aarch64` — run `cmake --preset <aarch64-preset>` to regenerate.*

```mermaid
graph TD
  subgraph lib_layer["Library Layer"]
    display("display")
    fmt("fmt")
    page_tables("page_tables")
  end
  subgraph subsys_layer["Subsystem Layer"]
    boot["boot"]
    drivers["drivers"]
    kmain["kmain"]
  end
  subgraph proto_layer["Protocol Libraries"]
    boot_limine[["boot_limine"]]
  end
  subgraph exec_layer["Executables"]
    kernel_limine_elf{{"kernel-limine.elf"}}
  end
  subgraph asm_layer["ASM Layer"]
    janus_asm[("janus_asm")]
  end
  display --> janus_asm
  fmt --> janus_asm
  page_tables --> janus_asm
  boot --> janus_asm
  boot --> display
  boot_limine --> boot
  drivers --> janus_asm
  drivers --> display
  drivers --> page_tables
  kmain --> janus_asm
  kmain --> drivers
  kmain --> boot
  kmain --> fmt
  kernel_limine_elf --> drivers
  kernel_limine_elf --> boot
  kernel_limine_elf --> fmt
  kernel_limine_elf --> boot_limine
  kernel_limine_elf --> kmain
```
