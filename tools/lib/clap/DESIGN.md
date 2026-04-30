# CLAP Design Document

**CLAP is Likely A Parser**
**Status:** Spec Complete — ready for pseudo-code phase
**Author:** Frederik Tobner
**Date:** 2026-04-30

---

## 1. Background

The JANUS toolchain is growing. Each tool — starting with `psf2c` — needs to accept, validate, and document its command-line arguments. The current approach in `psf2c` is ad-hoc: positional arguments are accessed by raw array index, the help text is a manually maintained string, and there is no standardised way to add optional flags or options with defaults. Every future tool would repeat this pattern with its own variation.

Rust's `clap` crate solves this problem elegantly for Rust programs. CLAP is a C-native answer to the same problem: a self-contained, stdlib-only library that lets a tool author describe their interface declaratively and get parsing, validation, and help generation for free.

---

## 2. Problem Statement

- Argument parsing in C tools is repetitive boilerplate with no shared infrastructure.
- Help text must be written and maintained by hand separately from the argument definitions, so they drift out of sync.
- There is no consistent user-facing error format across tools.
- Adding a new flag or option to any tool requires touching the parse loop, the help printer, and any validation logic independently.

---

## 3. Goals

- A single, reusable static library (`libclap`) that all tools in `tools/` link against.
- Arguments are **declared once**; parsing, validation, help text, and error messages all derive from that single declaration.
- The API should feel familiar to anyone who has used Rust's `clap`, while being idiomatic C11.
- Auto-generated `--help` output that includes the program name, version, a one-line description, and a formatted table of all arguments with their types, defaults, and descriptions.
- Auto-generated `--version` output.
- Consistent, human-readable error messages for every category of parse failure.
- The library must be usable with no external dependencies beyond the C standard library.

---

## 4. Non-Goals

- Subcommands (multi-verb CLIs) are explicitly out of scope for this version.
- Multi-value options (repeating the same flag multiple times to accumulate a list) are out of scope.
- Type coercion beyond strings is out of scope; callers convert values themselves.
- Shell autocompletion generation is out of scope.
- Internationalisation / localisation is out of scope.

---

## 5. Naming

The library is named **CLAP** — *CLAP is Likely A Parser*. This follows the tradition of recursive acronyms (GNU, WINE, LAME) where the first letter refers back to the name itself. The expansion is intentionally understated: "likely" acknowledges that whether something is truly a parser is, philosophically, a matter of perspective.

The folder name and CMake target name are lowercase `clap` for consistency with Unix conventions.

---

## 6. High-Level Design

### 6.1 Argument Kinds

Three kinds of argument are supported:

**Flag** — a boolean switch that is either present or absent. It takes no value. Flags always have a long form and optionally a short single-character form. Example: `--verbose` / `-v`.

**Option** — a named argument that takes exactly one string value. The value may be supplied as `--name value` or `--name=value`. Short forms are supported. An option may declare a default value and may be marked required. Example: `--output file.h` / `-o file.h`.

**Positional** — an argument identified by its position in the argument list rather than by a name. Positionals appear in the order they are declared. A positional may be required or optional; optional positionals must follow required ones. No positional can follow an optional positional.

### 6.2 Parser Definition

A parser is defined by two pieces of information held in static storage by the caller:

- **An argument descriptor array** — one entry per argument, each holding the kind, long name, optional short name, a human-readable metavar for values, a description, an optional default value, and a required flag.
- **A parser descriptor** — holds the program name, version string, and a one-line description of what the tool does, plus a reference to the argument array and its length.

Both are intended to be declared as static constants in the tool's source file and passed by pointer to the parsing function. The library does not take ownership of these structures.

### 6.3 Parse Results

After parsing, the library produces a result object that holds the resolved value for every argument (or the default if absent), and a boolean state for every flag. The result is heap-allocated by the library and freed by the caller through a dedicated free function.

Convenience lookup functions accept the result, the parser descriptor, and an argument name string, and return the resolved value or boolean state for that argument. Name lookup is linear over the descriptor array — the arrays are small and performance is irrelevant at tool startup.

### 6.4 Special Arguments

`--help` and `--version` are always recognised by the library, regardless of whether the caller declared them. On either flag, the library prints to standard output and calls `exit(0)`.

### 6.5 Error Handling

Parse errors — unknown flag, missing required argument, option without a value, value given to a flag — cause the library to print a short, structured message to standard error followed by a usage hint, then call `exit(1)`. There are no error codes returned to the caller. This is appropriate because argument errors are always fatal for command-line tools.

### 6.6 Help Format

The auto-generated help follows this structure:

- First line: program name and version.
- Second line: the one-line description.
- Blank line.
- A `Usage:` line showing the canonical invocation with positionals in angle brackets (required) or square brackets (optional), followed by `[OPTIONS]` if any flags or options exist.
- Blank line.
- An `Arguments:` section listing positionals in declaration order, with their metavar and description aligned in columns.
- An `Options:` section listing flags and named options, with short forms, long forms, metavars, defaults, and descriptions aligned in columns. `--help` and `--version` appear at the bottom of this section.

---

## 7. Acceptance Criteria

### Correctness

- All declared positionals are populated in declaration order from the unprefixed arguments on the command line.
- An option supplied as `--name value` and `--name=value` produce identical results.
- A flag's short form and long form are interchangeable.
- A declared default value is returned when an optional argument is absent.
- A required argument that is absent triggers an error and `exit(1)`.
- An unknown flag or option triggers an error and `exit(1)`.
- Passing `--help` prints help and exits with code 0 regardless of what other arguments are or are not present.
- Passing `--version` prints the version string and exits with code 0.

### Help Quality

- All declared arguments appear in the help output.
- Descriptions, metavars, and defaults are accurately reflected from the descriptor.
- The usage line correctly marks required positionals with `<>` and optional ones with `[]`.
- Long and short option forms are shown together.

### Integration

- `psf2c` compiles and behaves identically after being migrated to CLAP, verified by its existing test suite.
- No changes are required to the kernel build system; CLAP is only pulled in by tool targets.
- CLAP builds under GCC and Clang with `-Wall -Wextra -Wpedantic` and no warnings.
- CLAP has no dependency on anything outside the C standard library.

### Build System

- CLAP is built as a CMake static library target named `clap` under `tools/lib/clap/`.
- Any tool can link CLAP by adding a single `target_link_libraries` line.
- CLAP is not built when no tool requests it.

---

## 8. Implementation Steps (High Level)

1. **Create the library scaffold** — directory structure under `tools/lib/clap/`, CMakeLists.txt defining the static library target with public include paths.

2. **Define the public header** — all types (argument kind enum, argument descriptor struct, parser descriptor struct, result struct), all function declarations, and all declarative convenience macros. No implementation.

3. **Implement the core parser** — tokenise `argv`, match tokens against the descriptor array, populate result values, enforce required/optional constraints and positional ordering.

4. **Implement help generation** — compute column widths from descriptor data, then render the usage line, arguments section, and options section.

5. **Implement error reporting** — a shared internal function that formats and prints a category-specific message, appends the usage hint, and calls `exit(1)`.

6. **Implement result accessors and free** — the lookup functions and the heap cleanup function.

7. **Integrate into psf2c** — update `tools/psf2c/CMakeLists.txt` to link `clap`, replace the argument parsing and help printing in `main.c` with the CLAP API.

8. **Verify** — build psf2c, run the existing test suite, manually verify `--help` and `--version` output, and confirm that invalid invocations produce the expected error messages.

---

## 9. Resolved Decisions

| Question | Decision | Rationale |
|----------|----------|-----------|
| Support `--` end-of-options? | **Yes** | Any future tool taking file paths must not require users to avoid names starting with `-`. After `--`, all remaining tokens are unconditionally treated as positionals. |
| Fixed or adaptive column width? | **Fixed (24 characters)** | Adaptive width requires querying the terminal, adds complexity, and produces inconsistent output in CI logs and piped contexts. A fixed width is predictable everywhere. |
| `--help` / `--version` to stdout or stderr? | **stdout** | Help and version are requested output, not error output. Sending them to stdout allows `psf2c --help \| less` and script capture to work correctly. |
