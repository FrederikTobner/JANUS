
<p align="center"><a href="https://github.com/FrederikTobner/JANUS"><img src="./images/JANUS_Logo_Small.jpg" alt="JANUS Logo" height="180"/></a></p>
<p align="center">A monilitihic kernel, supporting x86_64 and aarch64.</p>

[![Build](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/build.yaml?branch=main&label=Build&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/build.yaml)
[![Tools](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/tools.yaml?branch=main&label=Tools&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/tools.yaml)
[![Doxygen](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/doxygen.yaml?branch=main&label=Doxygen&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/doxygen.yaml)
[![Coverage](https://img.shields.io/badge/coverage-see%20workflow-lightgrey?logo=codecov)](https://github.com/FrederikTobner/JANUS/actions/workflows/tools.yaml)
[![C Standard](https://img.shields.io/badge/C-C11%20freestanding-blue?logo=c)](https://en.cppreference.com/w/c/11)

## Quick Start

Building the kernel

```bash
# Using presets (recommended):
cmake --preset x86_64-gcc        # or x86_64-clang, aarch64-gcc, aarch64-clang
cmake --build --preset x86_64-gcc

# Or manually (defaults to x86_64 with system compiler):
cmake -B build -G Ninja
ninja -C build
```

Available presets: `x86_64-gcc`, `x86_64-clang`, `aarch64-gcc`, `aarch64-clang`

Creating all bootable ISO's for the current architecture with the supported boot protocols

```bash
ninja -C build iso         # or: cmake --build --preset <preset> --target iso
```

Running in QEMU using limine (both supported by x86_64 and aarch64)

```bash
ninja -C build run-limine         # or: cmake --build --preset <preset> --target run-limine
```

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
