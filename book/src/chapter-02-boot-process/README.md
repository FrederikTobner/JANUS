# Chapter 2: The Boot Process

> *"There are two ways of constructing a software design: One way is to make it so simple that there are obviously no deficiencies, and the other way is to make it so complicated that there are no obvious deficiencies."*  
> — C.A.R. Hoare

We have our tools installed. Time to write some code that talks directly to hardware. No operating system beneath us. No safety net. Just us, some assembly, and blind faith that GRUB will cooperate.

In this chapter, we'll:

- Set up the TinyOS project structure
- Write a Multiboot2-compliant bootloader header
- Create the boot entry assembly code
- Define memory layout with a linker script
- Automate building with CMake and Ninja
- Implement the kernel entry point in C

By the end of this chapter, we'll have a bootable kernel that verifies it was loaded correctly and halts gracefully.

**Note:** This kernel won't produce any visible output yet—that comes in the next chapter when we add serial I/O. For now, we're building the foundation that everything else depends on.

---

## Sections

1. [Project Structure](project-structure.md)
2. [The Multiboot2 Header](multiboot-header.md)
3. [Boot Entry Assembly](boot-assembly.md)
4. [The Linker Script](linker-script.md)
5. [The CMake Build System](build-system.md)
6. [The Kernel Entry Point](kernel-entry.md)
7. [Summary: What We've Built](summary.md)
