# clang-tidy

`clang-tidy` is JANUS' static-analysis gate for C code quality.

It runs in CI on the `x86_64-clang` build cell and should be run locally before opening a pull request that changes kernel code.

## What It Checks

The active checks are configured in `.clang-tidy` at repository root.

Typical categories include:

- readability
- bug-prone patterns
- portability and correctness

The project treats reported warnings as errors in CI for the kernel sweep.

## Local Usage

Configure first so `compile_commands.json` exists:

```bash
cmake --preset x86_64-clang -D CMAKE_BUILD_TYPE=Release
cmake --build --preset x86_64-clang
```

Run on all kernel C translation units:

```bash
jq -r '.[].file' build-x86_64-clang/compile_commands.json \
  | grep 'kernel/.*\.c$' \
  | xargs clang-tidy -p build-x86_64-clang
```

Run a single file:

```bash
clang-tidy -p build-x86_64-clang kernel/subsys/mm/src/pmm.c
```

## Workflow Tips

- Fix root causes rather than suppressing checks where possible.
- Keep functions short and single-purpose to avoid readability/function-size findings.
- Re-run `clang-format` after refactors to keep style and analysis aligned.
