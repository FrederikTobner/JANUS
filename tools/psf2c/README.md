# PSF2C

Tool that converts PSF (PC Screen Font) files to C headers for use in the kernel framebuffer driver, written in C.

## Building

```bash
# From the tools directory
mkdir build && cd build
cmake -G Ninja ..
ninja
```

## Running Tests

```bash
cd build
ctest --output-on-failure
```
