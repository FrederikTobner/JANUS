# Tools Philosophy

JANUS tools are programs that run on the developer's machine, not in the
kernel. They exist to automate steps that would otherwise have to be performed
manually during the build — converting font files, generating headers, validating
output, and similar offline work. The separation from the kernel is deliberate and
hard: tools never share source code with the kernel, and the kernel never shares
source code with the tools.

## Why a Separate Build

The kernel must be compiled with a freestanding cross-compiler; every
compiler flag, library restriction, and include-path rule follows from that
constraint. Tools run on the host and have no such constraints — they can
call `malloc`, use `printf`, link against system libraries, and assume
a reasonable hosted environment. Mixing the two into a single CMake project
would either import freestanding restrictions where they are unhelpful, or
create subtle ways for hosted APIs to leak into kernel code.

A clean cut also makes the build faster: configuring the kernel requires
downloading a cross-toolchain and a set of architecture-specific presets;
configuring the tools requires only a standard host C compiler.

## What Tools Are Allowed to Use

Tools have access to the full C11 hosted standard library and to any system
library that is available on the supported development platforms and that can
be declared as a CMake `find_package` dependency. There is no policy against
heap allocation, file I/O, or POSIX interfaces where they are available.

Third-party libraries are permitted at the tool level but must be declared
explicitly in CMake and must be reasonably portable. GoogleTest is used for
tool-level unit tests; it is fetched via `FetchContent` and never installed
into the system.

## What Tools Must Not Do

- **Include kernel headers.** The `kernel/include/` directory is off-limits.
  Shared concepts (e.g. font layout, header format) are defined in terms of
  plain C types, not kernel types.
- **Share source files with the kernel.** No `.c` file may be compiled both
  as part of a tool and as part of the kernel.
- **Depend on architecture-specific behaviour.** Tools must produce the same
  output regardless of whether the host is x86_64 or aarch64.

## Tool Libraries

Some logic is useful across multiple tools — argument parsing being the prime
example. Rather than duplicating code, shared logic lives in `tools/lib/` as
static libraries that tool executables link against. These libraries follow the
same rules as the tools themselves: hosted C11, no kernel headers.

A tool library may not depend on another tool executable. The dependency
graph is strictly acyclic and flows in one direction: executables depend on
libraries, not the reverse.

## Relationship to Kernel Philosophy

The tools share JANUS's emphasis on simplicity, narrow public interfaces, and
correctness over cleverness. Doxygen doc comments follow the same `@brief /
@param / @return` convention, the coding style (fixed-width integers, static
declarations before definitions) is the same, and every public API is as small
as it can be. The tools exist to serve the kernel, and they should be easy to
understand by anyone who already understands the kernel code.
