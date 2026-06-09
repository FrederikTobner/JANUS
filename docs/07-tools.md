# Developer Tools

Tools and tool libraries live in `tools/` and are built separately from the kernel
using a dedicated CMake project (`cmake -S tools -B build-tools`).

Tools may use the C standard library freely. They target the host platform, not
the kernel's freestanding environment.

## Tool Libraries (`tools/lib/`)

| Name | Description |
|---|---|
| clap | Declarative command-line argument parser (CLAP is Likely A Parser) |

## Tools

### psf2c

Converts PSF (PC Screen Font) files to C headers that compile directly into the
kernel. Supports PSF1 and PSF2 formats and handles gzip-compressed input (`.psf.gz`).

The kernel framebuffer text renderer needs glyph bitmaps at compile time. Embedding
font data as a C array avoids a filesystem requirement at early boot.

```
psf2c <input> <output> [prefix] [OPTIONS]

Arguments:
  <FILE>     Input PSF font file (.psf or .psf.gz)
  <FILE>     Output C header file
  [STRING]   Symbol prefix for generated identifiers [default: font]

Options:
  -v, --verbose   Print font information after conversion
      --help      Print help
      --version   Print version
```

The generated header defines:

- `<PREFIX>_WIDTH`, `<PREFIX>_HEIGHT`, `<PREFIX>_GLYPH_COUNT` — font dimensions
- `<PREFIX>_glyphs[]` — flat byte array of all glyph bitmaps in row-major order

Source layout:

```
tools/psf2c/
  src/
    main.c       argument parsing via clap
    psf.c/h      PSF1/PSF2 file parsing
    file_io.c/h  file loading with optional gzip decompression
    codegen.c/h  C header generation
  tests/         GoogleTest suite
  CMakeLists.txt
```

Dependencies: `clap`, `ZLIB`.

## Building Tools

```bash
cmake -S tools -B build-tools -G Ninja
cmake --build build-tools
ctest --test-dir build-tools --output-on-failure
```
