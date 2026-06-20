# clang-tidy Reference

## Prerequisites

clang-tidy reads from a `compile_commands.json`.
Always configure the `x86_64-clang` preset before running clang-tidy, because that preset produces the compilation database that clang-tidy expects:

```bash
cmake --preset x86_64-clang
cmake --build --preset x86_64-clang
```

## Running Against All Kernel Sources

```bash
jq -r '.[].file' build-x86_64-clang/compile_commands.json \
  | grep 'kernel/.*\.c$' \
  | xargs clang-tidy -p build-x86_64-clang
```

## Running on a Single File

```bash
clang-tidy -p build-x86_64-clang kernel/subsys/mm/src/pmm.c
```

## Running with Automatic Fixes

clang-tidy can apply some fixes automatically.
Always review the diff before committing:

```bash
clang-tidy -p build-x86_64-clang --fix kernel/subsys/mm/src/pmm.c
git diff
```

## Configuration

Active checks are controlled by `.clang-tidy` at the project root.
To see which checks are currently enabled:

```bash
clang-tidy --list-checks -p build-x86_64-clang kernel/subsys/mm/src/pmm.c \
  | grep -v "^Not"
```

## Suppressing a Finding

Suppressing a finding with `// NOLINT` should be a last resort.
Prefer fixing the root cause. When suppression is genuinely warranted, suppress the specific check by name rather than suppressing all checks on the line:

```c
// NOLINT(readability-magic-numbers)
#define VGA_TEXT_BUFFER_ADDR 0xB8000
```

## CI Equivalence

The `x86_64-clang` CI cell runs clang-tidy with the same compile database that is produced by the preset.
To reproduce a CI failure locally, configure the preset in Release mode (matching CI) and run clang-tidy:

```bash
cmake --preset x86_64-clang -D CMAKE_BUILD_TYPE=Release
cmake --build --preset x86_64-clang
jq -r '.[].file' build-x86_64-clang/compile_commands.json \
  | grep 'kernel/.*\.c$' \
  | xargs clang-tidy -p build-x86_64-clang
```
