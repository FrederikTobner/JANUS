# psf2c

`psf2c` converts PSF (PC Screen Font) font files into C headers that can be
compiled directly into the kernel. It supports both PSF1 and PSF2 formats and
handles gzip-compressed input (`.psf.gz`).

## Purpose

The kernel framebuffer text renderer needs glyph bitmaps at compile time, not
at runtime. Embedding font data as a C array avoids the need for a filesystem
at early boot and keeps the font resident in kernel BSS/data without additional
loading logic.

## Usage

```
psf2c <input> <output> [prefix] [OPTIONS]

Arguments:
  <FILE>     Input PSF font file (.psf or .psf.gz)
  <FILE>     Output C header file
  [STRING]   Symbol prefix for generated identifiers [default: font]

Options:
  -v, --verbose   Print font information after conversion
      --help      Print help
      --version   Print version information
```

The generated header defines:

- `<PREFIX>_WIDTH`, `<PREFIX>_HEIGHT`, `<PREFIX>_GLYPH_COUNT` — font dimensions as macros
- `<PREFIX>_glyphs[]` — a flat byte array of all glyph bitmaps in row-major order

## Source Layout

```
tools/psf2c/
  src/
    main.c       ← entry point; argument parsing via clap
    psf.c/h      ← PSF1/PSF2 file parsing
    file_io.c/h  ← file loading with optional gzip decompression
    codegen.c/h  ← C header generation
  tests/         ← GoogleTest suite for psf, file_io, and codegen
  CMakeLists.txt
```

## Dependencies

| Dependency | Purpose |
|------------|---------|
| `clap` | Command-line argument parsing |
| `ZLIB` | Gzip decompression of `.psf.gz` files |
