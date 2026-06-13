# CMake Reference

## Presets

The available presets are defined in `CMakePresets.json`. Each preset bundles the
generator, toolchain, build directory, and any preset-specific CMake variables.

```bash
# List all available presets
cmake --list-presets
cmake --build --list-presets

# Configure with a preset
cmake --preset x86_64-gcc

# Build with a preset
cmake --build --preset x86_64-gcc

# Run a specific target
cmake --build --preset x86_64-gcc --target run
cmake --build --preset x86_64-gcc --target iso
cmake --build --preset x86_64-gcc --target debug-limine
```

## Cache Inspection

```bash
# List all CMake variables with help strings
cmake --preset x86_64-gcc -LH

# Search the cache for JANUS-specific variables
grep JANUS build-x86_64-gcc/CMakeCache.txt

# Print the value of a single variable
cmake -L build-x86_64-gcc | grep JANUS_ARCH
```

## Verbose Build Output

```bash
# Print the full compiler command for every translation unit
cmake --build --preset x86_64-gcc --verbose

# Equivalent with Ninja directly
ninja -C build-x86_64-gcc -v
```

## Clean Rebuild

```bash
# Clean only (keeps CMake configuration)
cmake --build --preset x86_64-gcc --target clean

# Full clean — delete and reconfigure
rm -rf build-x86_64-gcc
cmake --preset x86_64-gcc
cmake --build --preset x86_64-gcc
```

## Debugging CMake Logic

```bash
# Print every CMake command as it executes, with variable expansions
cmake --preset x86_64-gcc --trace-expand

# Write the trace to a file instead of stderr
cmake --preset x86_64-gcc --trace-expand --trace-redirect=cmake-trace.log

# Enable CMake developer warnings (useful when writing new cmake/ modules)
cmake --preset x86_64-gcc --dev
```

## compile_commands.json

The compile database is written to the build directory during configure and copied
to the project root automatically. If clangd or clang-tidy reports stale results:

```bash
# Regenerate by reconfiguring
cmake --preset x86_64-gcc

# Verify the root symlink is current
ls -la compile_commands.json
```
