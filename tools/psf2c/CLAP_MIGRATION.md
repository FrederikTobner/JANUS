# psf2c — CLAP Migration Plan

**Status:** Pseudo-Code
**Depends on:** [tools/lib/clap/DESIGN.md](../lib/clap/DESIGN.md), [tools/lib/clap/IMPL.md](../lib/clap/IMPL.md)

This document specifies exactly how `psf2c/src/main.c` and
`psf2c/src/CMakeLists.txt` change when migrated to CLAP.
No existing logic outside `main.c` is touched.

---

## 1. What Changes

| File | Change |
|------|--------|
| `src/main.c` | Remove `print_usage`, remove manual index access, replace with CLAP descriptor + `clap_parse` |
| `src/CMakeLists.txt` | Add `add_subdirectory` for `clap`, add `target_link_libraries(psf2c PRIVATE clap)` |
| Everything else | Unchanged (`psf.c`, `codegen.c`, `file_io.c`, all tests) |

---

## 2. Argument Descriptor Declarations

These are static constants defined at file scope in `main.c`:

```
STATIC CONSTANT argument array ARGS:
    CLAP_POSITIONAL( name="input",  metavar="FILE",   help="Input PSF font file (.psf or .psf.gz)" )
    CLAP_POSITIONAL( name="output", metavar="FILE",   help="Output C header file" )
    CLAP_POSITIONAL_OPT( name="prefix", metavar="STRING", help="Symbol prefix", default="font" )
    CLAP_FLAG( name="verbose", short='v', help="Print font information after conversion" )

STATIC CONSTANT parser descriptor PARSER:
    name    = "psf2c"
    version = "0.1.0"
    about   = "Convert a PSF font file to a C header for use in the kernel"
    args    = ARGS
    nargs   = length of ARGS
```

---

## 3. `main` Logic After Migration

```
PROCEDURE main(argc, argv) → exit code

  result ← clap_parse(PARSER, argc, argv)
      // On --help:    prints help to stdout, exits 0   (handled inside clap_parse)
      // On --version: prints version to stdout, exits 0 (handled inside clap_parse)
      // On bad args:  prints error to stderr, exits 1   (handled inside clap_parse)

  input_path  ← clap_get(result, PARSER, "input")
  output_path ← clap_get(result, PARSER, "output")
  prefix      ← clap_get(result, PARSER, "prefix")   // never null; default is "font"
  verbose     ← clap_flag(result, PARSER, "verbose")

  clap_result_free(result)
      // Safe to free immediately: input_path/output_path/prefix point into
      // argv (owned by the OS for the lifetime of the process) or into the
      // static ARGS default string. Neither is freed by clap_result_free.

  // ── Load file ───────────────────────────────────────────────────────
  file_data ← null
  file_size ← 0
  IF file_load(input_path, &file_data, &file_size) is not success:
      RETURN 1

  // ── Parse PSF ───────────────────────────────────────────────────────
  font ← new psf_font_t
  IF psf_parse(file_data, file_size, &font) is not success:
      FREE file_data
      RETURN 1
  FREE file_data

  // ── Verbose output ───────────────────────────────────────────────────
  IF verbose:
      PRINT "Font info: {font.width}x{font.height}, {font.numglyphs} glyphs, {font.bytesperglyph} bytes/glyph"

  // ── Generate header ──────────────────────────────────────────────────
  fp_out ← open output_path for writing
  IF fp_out is null:
      PRINT to stderr: "error: cannot open output file '{output_path}'"
      psf_free(&font)
      RETURN 1

  codegen_write_header(fp_out, &font, prefix)
  CLOSE fp_out

  PRINT "Generated: {output_path}"

  psf_free(&font)
  RETURN 0

END PROCEDURE
```

---

## 4. Auto-Generated Help Output

After migration, `psf2c --help` will produce (rendered by CLAP from the
descriptor above):

```
psf2c 0.1.0
Convert a PSF font file to a C header for use in the kernel

Usage: psf2c <input> <output> [prefix] [OPTIONS]

Arguments:
  <FILE>                  Input PSF font file (.psf or .psf.gz)
  <FILE>                  Output C header file
  [STRING]                Symbol prefix [default: font]

Options:
  -v, --verbose           Print font information after conversion
      --help              Print help
      --version           Print version information
```

---

## 5. CMake Changes

### `src/CMakeLists.txt` additions

```
Add subdirectory ../../lib/clap with binary dir clap-build
    (relative path from psf2c/src to tools/lib/clap)

In the psf2c executable target's link libraries:
    Add clap with PRIVATE visibility
```

The `psf2c_lib` static library (used by tests) does NOT link clap — the
argument parsing only lives in `main.c`, not in the shared library.

---

## 6. Behaviour Parity

The following table confirms that the migrated psf2c is observably identical
to the original for all valid and invalid invocations:

| Invocation | Original behaviour | Migrated behaviour |
|------------|-------------------|-------------------|
| `psf2c a.psf b.h` | runs with prefix "font" | runs with prefix "font" |
| `psf2c a.psf b.h myfont` | runs with prefix "myfont" | runs with prefix "myfont" |
| `psf2c a.psf` | prints usage to stderr, exits 1 | prints error + hint to stderr, exits 1 |
| `psf2c` | prints usage to stderr, exits 1 | prints error + hint to stderr, exits 1 |
| `psf2c --help` | prints usage to stderr, exits 1 | prints full help to stdout, exits 0 |
| `psf2c --version` | not supported (unknown option) | prints "psf2c 0.1.0" to stdout, exits 0 |
| `psf2c --unknown` | not handled | prints error + hint to stderr, exits 1 |
| `psf2c a.psf b.h -- --odd-name.h` | not supported | treats `--odd-name.h` as a positional |

The one deliberate behaviour improvement is `--help`: the original prints
to stderr and exits 1. The migrated version prints full structured help to
stdout and exits 0, which is the standard Unix convention.
