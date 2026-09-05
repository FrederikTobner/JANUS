<p align="center"><a href="https://github.com/FrederikTobner/JANUS"><img src="./images/JANUS_Logo_Small.jpg" alt="JANUS Logo" height="180"/></a></p>
<p align="center">A monolithic kernel, supporting x86_64 and aarch64.</p>

[![Build](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/build.yaml?branch=main&label=Build&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/build.yaml)
[![Tools](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/tools.yaml?branch=main&label=Tools&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/tools.yaml)
[![Doxygen](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/doxygen.yaml?branch=main&label=Doxygen&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/doxygen.yaml)
[![Smoke](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/smoke.yaml?branch=main&labelSmoke&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/smoke.yaml)
[![Coverage](https://codecov.io/gh/FrederikTobner/JANUS/graph/badge.svg)](https://codecov.io/gh/FrederikTobner/JANUS)
[![C Standard](https://img.shields.io/badge/C-C17%20freestanding-blue?logo=c)](https://en.cppreference.com/w/c/17)

## Quick Start

For building the kernel the usage of one of the presets is recommended to simplify the configuration.

```bash
# Using presets (recommended):
cmake --preset x86_64-gcc        # or x86_64-clang, aarch64-gcc, aarch64-clang
cmake --build --preset x86_64-gcc
```

Currently the following presets are defined:

| Architecture | Compiler | Preset name  |
|--------------|----------|--------------|
| x86_64       | clang    | x86_64-clang |
| x86_64       | gcc      | x86_64-gcc   |
| aarch64      | clang    | aarch64-clang |
| aarch64      | gcc      | aarch64-gcc   |

For creating all bootable ISO's for the current architecture with the supported boot protocols, the following command can be used:

```bash
ninja -C build-<preset> iso         
```

For running the kernel in QEMU every supported boot protocol defines its own target:

```bash
ninja -C build-<preset> run-<protocol>         # E.g ninja -C build-x86_64-gcc run-limine
```

JANUS supports the following boot protocols (per cpu architecture):

| Architecture | Supported Boot Protocols |
|--------------|--------------------------|
| x86_64       | limine, multiboot2       |
| aarch64      | limine                   |

A more detailed setup guide can be found in the [documentation](./docs/01-getting-started.md)

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
