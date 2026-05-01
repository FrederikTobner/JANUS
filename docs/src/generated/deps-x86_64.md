> *Generated for `x86_64` — run `cmake --preset <x86_64-preset>` to regenerate.*

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
    boot_multiboot2[["boot_multiboot2"]]
  end
  subgraph exec_layer["Executables"]
    kernel_limine_elf{{"kernel-limine.elf"}}
    kernel_multiboot2_elf{{"kernel-multiboot2.elf"}}
  end
  subgraph asm_layer["ASM Layer"]
    janus_asm[("janus_asm")]
  end
  page_tables --> janus_asm
  boot --> display
  boot_limine --> boot
  boot_multiboot2 --> boot
  drivers --> display
  drivers --> page_tables
  kmain --> drivers
  kmain --> boot
  kmain --> fmt
  kernel_limine_elf --> drivers
  kernel_limine_elf --> boot
  kernel_limine_elf --> fmt
  kernel_limine_elf --> boot_limine
  kernel_limine_elf --> kmain
  kernel_multiboot2_elf --> drivers
  kernel_multiboot2_elf --> boot
  kernel_multiboot2_elf --> fmt
  kernel_multiboot2_elf --> boot_multiboot2
  kernel_multiboot2_elf --> kmain
```
