# Contract layer

Used to explicitly declare dependencies between subsystems.

## Contracts

| Contract  | Consumers      | Purpose                                             |
|-----------|----------------|-----------------------------------------------------|
| `memmap`  | boot, mm       | Shared memory-map entry type (`memmap_entry_t`)     |
| `display` | boot, drivers  | Shared display config types (`display_info_t`, `display_mode_t`) |
