# clap — CLAP is Likely A Parser

`clap` is a small, declarative command-line argument parsing library for C
tool executables. It is the tools-space equivalent of the popular Rust
`clap` crate in spirit, but is implemented from scratch in C11 with no
external dependencies.

## Purpose

Every tool needs to parse `argc`/`argv`, validate required arguments, print
consistent help text, and exit cleanly on errors. Writing this logic from
scratch in each tool leads to duplicated code and inconsistent error messages.
`clap` centralises that contract so all tools behave the same way.

## Public API

The entire public interface is exposed through the single header
`<clap/clap.h>`. The workflow is:

1. Declare a `static clap_arg_t const ARGS[]` array using the six declarative
   macros (`CLAP_FLAG`, `CLAP_OPTION`, `CLAP_OPTION_DEFAULT`, `CLAP_REQUIRED`,
   `CLAP_POSITIONAL`, `CLAP_POSITIONAL_OPT`).
2. Declare a `static clap_parser_t const PARSER` with the tool name, version,
   about string, and the argument array.
3. Call `clap_parse(&PARSER, argc, argv)` — the library handles `--help`,
   `--version`, unknown flags, and missing required arguments, exiting
   appropriately in each case.
4. Retrieve values with `clap_get()` and flags with `clap_flag()`.
5. Free the result with `clap_result_free()`.

## Built-in Behaviour

`--help` and `--version` are built in and do not need to appear in the
argument array. `clap_parse` handles them before looking at user-defined
arguments. `--` is also handled: anything after it is treated as a positional
regardless of whether it looks like a flag.

Error messages are written to stderr; help and version output goes to stdout.
All three paths call `exit()` — tools are expected to have a single clean exit
point and argument errors are always fatal.

## Source Layout

```
tools/lib/clap/
  include/clap/clap.h   ← public API
  src/help.h            ← internal declarations shared between clap.c and help.c
  src/clap.c            ← parse loop, validation, accessors, memory management
  src/help.c            ← help and version rendering
  CMakeLists.txt
```

