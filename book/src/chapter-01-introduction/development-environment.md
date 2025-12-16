# Development Environment

We develop on Linux (or WSL2 on Windows). TinyOS targets x86_64, so you'll need:

[!side]Why Clang instead of GCC? Better error messages, cleaner cross-compilation, and LLDB integration. But GCC works too if you prefer it.[/!side]

- **Clang** (17+) for C compilation
- **NASM** for boot assembly  
- **QEMU** for virtual machine testing
- **LLDB** for debugging
- **CMake** (3.20+) and **Ninja** for builds

Chapter 2 covers installation in detail.

## Learning Paths

**Systems programming background (C, Rust, Go):**  
Focus on OS-specific concepts—paging, interrupts, hardware interaction. You might move faster through early chapters.

[!side]Coming from high-level languages? The lack of safety rails will feel uncomfortable. No garbage collector, no bounds checking, no stack traces. Spend extra time on debugging sections.[/!side]

**High-level languages (Python, JavaScript, Java):**  
Spend extra time on C fundamentals and debugging sections. Consider working through a C programming book alongside this one. Take breaks when frustrated.

**Students:**  
This complements OS theory courses—hands-on implementation makes abstract concepts concrete. Use challenges to deepen understanding beyond exams.

**Self-taught:**  
The incremental approach is your friend. Each chapter builds working code. Stop after any chapter—no pressure to finish everything.

---

**Next: [How to Use This Book](how-to-use.md)**
