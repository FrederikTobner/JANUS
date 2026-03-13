# JANUS C Programming Standards

This project uses only C in combination with assembly (both inline and sepeate), at least for the kernel.
Other layers in the OS might use a different language, but C is still the default.

## Programming Languages

### C17 (ISO/IEC 9899:2018)

JANUS uses C17 (ISO/IEC 9899:2018) as specified in `.clang-format`:

### Assembly

Additionally we use NASM with the intel dialect for x86/x64 architectures and GAS with the AT&T dialect for ARM64.
