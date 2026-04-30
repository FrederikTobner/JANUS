# CLAP Implementation Pseudo-Code

**Status:** Pseudo-Code
**Follows:** [DESIGN.md](DESIGN.md)

This document specifies the complete logic of the CLAP library using structured
pseudo-code. No actual C syntax is used. The intent is to be precise enough
that translating each section to C is mechanical.

---

## 1. File & Directory Layout

```
tools/lib/clap/
  CMakeLists.txt
  include/
    clap/
      clap.h          ← entire public API: types, macros, function declarations
  src/
    internal.h        ← shared internal declarations (not installed, not public)
    clap.c            ← parse loop, validation, accessors, memory, error reporting
    help.c            ← print_help and print_version (text rendering only)
  DESIGN.md
  IMPL.md             ← this file
```

`clap.h` is the only file a consumer includes. The implementation is split
along a natural seam: `clap.c` is pure logic (no output formatting beyond
error strings), and `help.c` is pure output rendering. They share
declarations through `internal.h`, which is not part of the installed
interface.

The split keeps each file focused and around 150–200 lines — small enough to
read in one sitting, without fragmenting trivially small pieces into their
own files.

---

## 2. Public Types

### 2.1 Argument Kind

An enumeration with three values:

- `CLAP_FLAG`       — boolean switch, no value
- `CLAP_OPTION`     — named argument, takes one string value
- `CLAP_POSITIONAL` — position-identified argument, takes one string value

### 2.2 Argument Descriptor (`clap_arg_t`)

One instance per argument. Intended as a static constant.

| Field          | Type            | Meaning |
|----------------|-----------------|---------|
| `kind`         | argument kind   | Which of the three kinds this argument is |
| `name`         | string          | Long name without `--`. Also the lookup key and the positional display name. |
| `short_name`   | character       | Single character for the short form. Zero means no short form. |
| `metavar`      | string          | Placeholder shown in help for the value, e.g. `FILE`. Ignored for flags. |
| `help`         | string          | One-line description shown in the Options or Arguments section. |
| `default_val`  | string or null  | Value used when the argument is absent. Null means no default. |
| `required`     | boolean         | If true, absence after parsing triggers a fatal error. |

### 2.3 Parser Descriptor (`clap_parser_t`)

Describes the whole program. Intended as a static constant.

| Field    | Type            | Meaning |
|----------|-----------------|---------|
| `name`   | string          | Executable name shown in Usage and header lines. |
| `version`| string          | Version string shown by `--version` and in the help header. |
| `about`  | string          | One-line description of the tool. |
| `args`   | pointer to array of `clap_arg_t` | The argument descriptor array. |
| `nargs`  | integer         | Length of the `args` array. |

### 2.4 Parse Result (`clap_result_t`)

Heap-allocated by `clap_parse`. Parallel arrays over `parser->args`.

| Field    | Type               | Meaning |
|----------|--------------------|---------|
| `values` | array of strings   | For each arg at index `i`: the parsed or default value, or null if absent and optional. Valid for `CLAP_OPTION` and `CLAP_POSITIONAL`. Undefined (null) for `CLAP_FLAG`. |
| `flags`  | array of booleans  | For each arg at index `i`: true if the flag was present. Valid for `CLAP_FLAG`. Undefined (false) for other kinds. |

Both arrays have length equal to the `nargs` of the parser used to produce the result.

---

## 3. Declarative Macros

All macros expand to a `clap_arg_t` compound literal. They are the intended
way to fill the argument descriptor array.

| Macro | Kind | required | short_name | default_val |
|-------|------|----------|------------|-------------|
| `CLAP_FLAG(long, short, help)` | FLAG | false | `short` | null |
| `CLAP_OPTION(long, short, meta, help)` | OPTION | false | `short` | null |
| `CLAP_OPTION_DEFAULT(long, short, meta, help, def)` | OPTION | false | `short` | `def` |
| `CLAP_REQUIRED(long, short, meta, help)` | OPTION | true | `short` | null |
| `CLAP_POSITIONAL(name, meta, help)` | POSITIONAL | true | 0 | null |
| `CLAP_POSITIONAL_OPT(name, meta, help, def)` | POSITIONAL | false | 0 | `def` |

A short name of `0` signals "no short form". Macros that take no short form
hardcode zero.

---

## 4. Public Function Declarations

```
clap_result_t * clap_parse(clap_parser_t const * parser, int argc, char ** argv)
```

Parse `argv` against `parser`. Exits on `--help`, `--version`, or any parse
error. Returns a heap-allocated result on success.

```
char const * clap_get(clap_result_t const * result,
                      clap_parser_t const * parser,
                      char const * name)
```

Return the resolved string value for the argument named `name`. Returns null
if the argument was absent and has no default. Behaviour is undefined if
`name` does not match any argument in `parser`.

```
bool clap_flag(clap_result_t const * result,
               clap_parser_t const * parser,
               char const * name)
```

Return true if the flag named `name` was present. Behaviour is undefined if
`name` does not match a `CLAP_FLAG` argument in `parser`.

```
void clap_result_free(clap_result_t * result)
```

Free the result and its internal arrays. Does not free string contents because
values point into the original `argv` or into the static descriptor defaults.

---

## 5. Internal Helper Procedures

These are not exposed in the public header.

### 5.1 `find_by_long(parser, name) → index or NOT_FOUND`

```
FOR i FROM 0 TO parser.nargs - 1:
    IF parser.args[i].name equals name:
        RETURN i
RETURN NOT_FOUND
```

### 5.2 `find_by_short(parser, ch) → index or NOT_FOUND`

```
FOR i FROM 0 TO parser.nargs - 1:
    IF parser.args[i].short_name equals ch:
        RETURN i
RETURN NOT_FOUND
```

### 5.3 `find_nth_positional(parser, n) → index or NOT_FOUND`

```
count ← 0
FOR i FROM 0 TO parser.nargs - 1:
    IF parser.args[i].kind equals CLAP_POSITIONAL:
        IF count equals n:
            RETURN i
        INCREMENT count
RETURN NOT_FOUND
```

### 5.4 `clap_die(parser, fmt, ...) → does not return`

```
PRINT to stderr: "error: " followed by the formatted message
PRINT to stderr: "Try '" + parser.name + " --help' for more information."
EXIT with code 1
```

---

## 6. Core Parser Algorithm (`clap_parse`)

```
PROCEDURE clap_parse(parser, argc, argv) → result

  // Allocate result
  result ← new clap_result_t
  result.values ← new array of (parser.nargs) null string pointers
  result.flags  ← new array of (parser.nargs) false booleans

  // Apply defaults before parsing so any absent optional argument
  // already has its default value without needing special casing later.
  FOR i FROM 0 TO parser.nargs - 1:
      IF parser.args[i].default_val IS NOT NULL:
          result.values[i] ← parser.args[i].default_val

  // Main parse loop
  end_of_options    ← false
  positional_cursor ← 0   // counts how many positionals have been filled
  i                 ← 1   // start at 1 to skip argv[0] (program name)

  WHILE i < argc:
      token ← argv[i]

      // ── Case 1: bare "--" ───────────────────────────────────────────
      IF NOT end_of_options AND token equals "--":
          SET end_of_options ← true
          ADVANCE i
          CONTINUE

      // ── Case 2: long option/flag ("--name" or "--name=value") ───────
      ELSE IF NOT end_of_options AND token starts with "--":
          raw ← token with leading "--" stripped

          IF raw contains "=":
              // "--name=value" form
              name  ← substring of raw before the first "="
              value ← substring of raw after the first "="
              idx ← find_by_long(parser, name)
              IF idx equals NOT_FOUND:
                  clap_die(parser, "unknown option: --%s", name)
              IF parser.args[idx].kind equals CLAP_FLAG:
                  clap_die(parser, "flag --%s does not accept a value", name)
              result.values[idx] ← value

          ELSE:
              // "--name" form (flag or option expecting next token as value)
              IF raw equals "help":
                  CALL print_help(parser) → stdout
                  EXIT 0
              IF raw equals "version":
                  CALL print_version(parser) → stdout
                  EXIT 0
              idx ← find_by_long(parser, raw)
              IF idx equals NOT_FOUND:
                  clap_die(parser, "unknown option: --%s", raw)
              IF parser.args[idx].kind equals CLAP_FLAG:
                  result.flags[idx] ← true
              ELSE (CLAP_OPTION):
                  ADVANCE i
                  IF i >= argc:
                      clap_die(parser, "option --%s requires a value", raw)
                  result.values[idx] ← argv[i]

      // ── Case 3: short option/flag ("-x" or "-x value") ──────────────
      ELSE IF NOT end_of_options AND token starts with "-" AND length(token) > 1:
          ch ← second character of token  // token[1]
          IF ch equals 'h':
              CALL print_help(parser) → stdout
              EXIT 0
          idx ← find_by_short(parser, ch)
          IF idx equals NOT_FOUND:
              clap_die(parser, "unknown option: -%c", ch)
          IF parser.args[idx].kind equals CLAP_FLAG:
              result.flags[idx] ← true
          ELSE (CLAP_OPTION):
              ADVANCE i
              IF i >= argc:
                  clap_die(parser, "option -%c requires a value", ch)
              result.values[idx] ← argv[i]

      // ── Case 4: positional argument ──────────────────────────────────
      ELSE:
          pos_idx ← find_nth_positional(parser, positional_cursor)
          IF pos_idx equals NOT_FOUND:
              clap_die(parser, "unexpected argument: %s", token)
          result.values[pos_idx] ← token
          INCREMENT positional_cursor

      ADVANCE i
  END WHILE

  // Validate: all required arguments must be resolved
  FOR i FROM 0 TO parser.nargs - 1:
      arg ← parser.args[i]
      IF arg.required AND result.values[i] IS NULL:
          IF arg.kind equals CLAP_POSITIONAL:
              clap_die(parser, "missing required argument <%s>", arg.name)
          ELSE:
              clap_die(parser, "missing required option --%s", arg.name)

  RETURN result

END PROCEDURE
```

---

## 7. Help Generation (`print_help`)

```
PROCEDURE print_help(parser) → stdout

  CONSTANT HELP_COL ← 24   // fixed column width for the left-side entries

  // ── Header ──────────────────────────────────────────────────────────
  PRINT "{parser.name} {parser.version}"
  PRINT "{parser.about}"
  PRINT blank line

  // ── Usage line ──────────────────────────────────────────────────────
  usage ← "Usage: " + parser.name
  FOR each arg IN parser.args WHERE arg.kind equals CLAP_POSITIONAL, in declaration order:
      IF arg.required:
          APPEND " <" + arg.name + ">" to usage
      ELSE:
          APPEND " [" + arg.name + "]" to usage
  IF any arg in parser.args has kind CLAP_FLAG or CLAP_OPTION:
      APPEND " [OPTIONS]" to usage
  PRINT usage
  PRINT blank line

  // ── Arguments section (positionals only) ────────────────────────────
  IF any arg in parser.args has kind CLAP_POSITIONAL:
      PRINT "Arguments:"
      FOR each arg IN parser.args WHERE arg.kind equals CLAP_POSITIONAL, in declaration order:
          IF arg.required:
              left ← "  <" + arg.metavar + ">"
          ELSE:
              left ← "  [" + arg.metavar + "]"
          description ← arg.help
          IF arg.default_val IS NOT NULL:
              APPEND " [default: " + arg.default_val + "]" to description
          PRINT left padded to HELP_COL + description
      PRINT blank line

  // ── Options section (flags and named options) ────────────────────────
  PRINT "Options:"
  FOR each arg IN parser.args WHERE arg.kind equals CLAP_FLAG or CLAP_OPTION, in declaration order:
      IF arg.short_name is not zero:
          left ← "  -" + arg.short_name + ", --" + arg.name
      ELSE:
          left ← "      --" + arg.name
      IF arg.kind equals CLAP_OPTION:
          APPEND " <" + arg.metavar + ">" to left
      description ← arg.help
      IF arg.default_val IS NOT NULL:
          APPEND " [default: " + arg.default_val + "]" to description
      PRINT left padded to HELP_COL + description

  // Built-in entries always printed last
  PRINT "      --help" padded to HELP_COL + "Print help"
  PRINT "      --version" padded to HELP_COL + "Print version information"

END PROCEDURE
```

**Padding rule:** If `left` is shorter than `HELP_COL`, pad with spaces to
reach column `HELP_COL` before printing the description. If `left` is longer,
insert a single space before the description (overflow gracefully, no wrapping).

---

## 8. Version Output (`print_version`)

```
PROCEDURE print_version(parser) → stdout
  PRINT "{parser.name} {parser.version}"
END PROCEDURE
```

---

## 9. Result Accessors

### `clap_get`

```
PROCEDURE clap_get(result, parser, name) → string or null
  FOR i FROM 0 TO parser.nargs - 1:
      IF parser.args[i].name equals name:
          RETURN result.values[i]
  RETURN null   // name not found; caller should not reach this with a valid name
END PROCEDURE
```

### `clap_flag`

```
PROCEDURE clap_flag(result, parser, name) → boolean
  FOR i FROM 0 TO parser.nargs - 1:
      IF parser.args[i].name equals name:
          RETURN result.flags[i]
  RETURN false  // name not found
END PROCEDURE
```

---

## 10. Memory Management (`clap_result_free`)

```
PROCEDURE clap_result_free(result)
  // Values point into argv (caller owns) or into static default_val strings.
  // Neither must be freed here. Only the arrays themselves and the struct
  // are heap-allocated by clap_parse.
  FREE result.values
  FREE result.flags
  FREE result
END PROCEDURE
```

---

## 11. CMake Setup

### `tools/lib/clap/CMakeLists.txt`

```
Define a static library target named "clap"
  Sources:   src/clap.c
  Public include path: include/   (exposes clap/clap.h to consumers)
  C standard: C11
  Compile with: -Wall -Wextra -Wpedantic
```

### Consumer integration (any tool)

```
Add subdirectory tools/lib/clap (with a binary dir argument to avoid conflicts)
Link the tool's executable target against "clap" (PRIVATE linkage)
```

The tool then just includes `<clap/clap.h>` and uses the API. No other
setup is required.
