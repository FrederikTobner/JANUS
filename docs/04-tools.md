# Developer Tools

Tools and tool libraries live in `tools/` and are built as a separate CMake project, from the tools folder rather than the root.
Because they target the host platform rather than the freestanding kernel environment, they may use the full C standard library and any host libraries they need.
Building the tools is entirely independent of building the kernel:

```bash
cmake -S tools -B build-tools -G Ninja
cmake --build build-tools
ctest --test-dir build-tools --output-on-failure
```

More information about the available tools can be found in the readme at the root level.
