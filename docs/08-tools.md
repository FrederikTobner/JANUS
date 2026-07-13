# Developer Tools

Tools and tool libraries live in `tools/` and are built as a separate CMake project.
Because they target the host platform rather than the freestanding kernel environment, they may use the full C standard library and any host libraries they need.
Building the tools is entirely independent of building the kernel:

```bash
cmake -S tools -B build-tools -G Ninja
cmake --build build-tools
ctest --test-dir build-tools --output-on-failure
```

## Tool Libraries (`tools/lib/`)

| Name                                | Description                                                        |
|-------------------------------------|--------------------------------------------------------------------|
| [clap](../tools/lib/clap/README.md) | Declarative command-line argument parser (CLAP is Likely A Parser) |

## psf2c

Tool that converts PSF (PC Screen Font) files to C headers for use in the kernel framebuffer driver, supporting PSF1 and PSF2 format variants and gzip-compressed input.

More information regarding the tool can be found in its [readme](../tools/psf2c/README.md),
