# PSF2C

`psf2c` converts PC Screen Font files (`.psf`, `.psf.gz`) into C headers that compile directly into the kernel binary.
The tool supports PSF1 and PSF2 format variants and gzip-compressed input, allowing the user to embed a complete bitmap font at compile time without requiring a filesystem or a runtime font loading step.

The kernel framebuffer text renderer uses the generated header directly.
At link time the glyph data is part of the image, resident in memory as soon as the kernel s loaded, with no indirection and no dynamic allocation.

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

The generated header defines three dimension macros and one glyph array:

- `<PREFIX>_WIDTH`, `<PREFIX>_HEIGHT` — dimensions in pixels of a single glyph
- `<PREFIX>_GLYPH_COUNT` — number of glyphs in the font
- `<PREFIX>_glyphs[]` — flat byte array of all glyph bitmaps in row-major order

More information regarding the tool can be found in its [readme](../tools/psf2c/README.md),
