# JANUS Build and Utility Scripts

This directory contains lua scripts for building, testing, and running JANUS.

## Content

* `build.lua` - Builds the kernel and creates an ISO using the permutations of all supported compilers, boot protocols and hardware architectures.
* `format.lua` - Formats the kernel and tools sources using the rules defined in the root .clang-format file
* `tidy.lua` - Performs static analysis on the kernel using the rules defined in kernel/.clang-tidy
* `lib/` - Shared Lua modules used by the scripts above (colour output, shell execution, CLI diagnostics, project-root resolution, timing, progress reporting, toolchain discovery).
