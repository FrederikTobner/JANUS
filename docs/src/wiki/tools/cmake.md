# CMake

CMake is a build system generator — it does not compile code itself, but produces build files for a backend like Ninja or Make. JANUS uses CMake 3.20+ with Ninja as the default backend.

## Presets

CMake presets (defined in `CMakePresets.json`) bundle the generator, toolchain file, and build directory into a single name:

```bash
cmake --preset x86_64-gcc          # Configure
cmake --build --preset x86_64-gcc  # Build
cmake --build --preset x86_64-gcc --target run  # Build + run
```

Each preset uses its own build directory (`build-x86_64-gcc/`, etc.), so multiple configurations coexist without conflict.

## Essential Commands

### Configure and Build

```bash
cmake -B build -G Ninja                     # Configure with Ninja
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-gcc.cmake
cmake --build build                         # Build (or: ninja -C build)
cmake --build build --target iso            # Build a specific target
cmake --build build -- -j$(nproc)           # Parallel build
```

### Inspect the Configuration

```bash
cmake -B build -G Ninja -LA                 # List all cache variables
cmake -B build -G Ninja -LH                 # List with help strings
cat build/CMakeCache.txt | grep JANUS       # Grep project-specific variables
```

### Clean Rebuild

```bash
cmake --build build --target clean          # Remove built objects
rm -rf build && cmake -B build -G Ninja     # Full reconfigure
```

## Key CMake Concepts

**Targets** are the central abstraction. A target is a library or executable with associated sources, include paths, compile options, and link dependencies. JANUS defines targets through custom helpers (`janus_add_subsys`, `janus_add_library`) that also set up architecture include paths.

**`INTERFACE` libraries** export include paths and compile definitions to consumers but compile no sources of their own. JANUS uses this for header-only modules (e.g., the `boot` subsystem's public headers).

**`PUBLIC` vs. `PRIVATE` dependencies.** A `PUBLIC` dependency propagates to consumers of the target — anyone linking against target A also gets target B's include paths. A `PRIVATE` dependency is internal: consumers of A do not see B. JANUS uses this distinction to enforce the layer model.

**Generator expressions** (`$<...>`) are evaluated at generate time, not configure time. They are used throughout the JANUS build system to conditionally set flags based on compiler identity, architecture, or build type.

**Toolchain files** set the compiler, linker, and system root for cross-compilation. JANUS provides toolchain files under `cmake/toolchains/` for all four preset configurations.

## Debugging CMake Issues

```bash
cmake -B build -G Ninja --trace-expand      # Full trace of CMake execution
cmake -B build -G Ninja --debug-output      # Debug-level output
cmake --build build --verbose               # Show raw compiler commands
```

The `compile_commands.json` generated in the build directory is consumed by clangd for IDE support. If autocomplete is broken, check that this file exists and is current.
