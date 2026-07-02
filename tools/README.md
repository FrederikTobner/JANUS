# JANUS Development Tools

Development tools and utilities built separately from the kernel using their own CMake project.

## Tools

### psf2c

Converts PSF (PC Screen Font) files to C headers for embedding fonts directly
in the kernel framebuffer driver.
See [psf2c/README.md](psf2c/README.md).

## Libraries

### clap

Declarative command-line argument parsing library for C tools.
Provides flag, option, and positional argument parsing with auto-generated help output and `--version` support.
See [lib/clap/README.md](lib/clap/README.md).

## Building

```bash
cmake --preset tools-release   # or tools-debug
cmake --build --preset tools-release
```

Binaries are placed under `tools/build/`.
