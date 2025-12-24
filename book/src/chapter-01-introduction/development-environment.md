# Development Environment

We develop on Linux (or WSL2 on Windows). TinyOS targets x86_64, so you'll need:

[!side]Why Clang instead of GCC? Better error messages, cleaner cross-compilation, and LLDB integration. But GCC works too if you prefer it.[/!side]

- **Clang** (17+) for C compilation
- **NASM** for boot assembly  
- **QEMU** for virtual machine testing
- **LLDB** for debugging
- **CMake** (3.20+) 
- **Ninja** for builds

Chapter 2 covers installation in detail.

---

**Next: [How to Use This Book](how-to-use.md)**
