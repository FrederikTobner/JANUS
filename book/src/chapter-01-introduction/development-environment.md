# Development Environment

We develop on Linux (or WSL2 on Windows). TinyOS targets x86_64, so you'll need:

[!side]Why are we using the tools provided by LLVM instead of GCC? LLVM offers a better error messages from the compiler, but you can also use GCC if you prefer. Since Clang is a drop-in replacement for GCC, the code we write will be compatible with both compilers.
[/!side] 

> TODO: Actually test compilation using GCC once we finished the book.

- **Clang** (17+) for C compilation
- **NASM** for boot assembly  
- **QEMU** for virtual machine testing
- **LLDB** for debugging
- **CMake** (3.20+) 
- **Ninja** for builds

> TODO: Ensure cmake 3.20 would work or update to the oldest tested version (at least 3.30.5 works)


Chapter 2 covers installation in detail.

---

**Next: [How to Use This Book](how-to-use.md)**
