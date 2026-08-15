# JANUS Build and Utility Scripts

This directory contains shell scripts for building, testing, and running JANUS.

## Content

* `build.lua` - Builds the kernel and creates an ISO using the permutations of all supported compilers, boot protocols and hardware architectures.
* `format.lua` - Formats all the files in the project using the rules defined in the .clang-format file
* `tidy.lua` - Performs static analysis on the project using the rules defined in the .clang-tidy file
* `lib/` - Shared Lua modules used by the scripts above (colour output, shell execution, CLI diagnostics, project-root resolution, timing, progress reporting, toolchain discovery).
