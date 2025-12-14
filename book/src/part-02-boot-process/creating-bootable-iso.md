# Creating a Bootable ISO

We have a complete kernel: Multiboot2 header, boot assembly, linker script, C entry point, and CMake build system. Everything compiles and links successfully producing a `kernel.elf` file. But how do we actually *boot* it?

Here's the problem: QEMU can't just boot our ELF file. It expects either a Linux-style bzImage or a special PVH boot protocol. Our kernel speaks Multiboot2, which requires a bootloader. Enter GRUB.

> **The Crux: Why Can't We Just Load the Kernel Directly?**
>
> You might wonder why we need this extra step. The kernel is right there! But real hardware doesn't know how to parse ELF files. GRUB acts as the middleman: it understands ELF, sets up the environment our kernel expects (64-bit mode, Multiboot info structure, etc.), and hands control over. Think of it as a helpful librarian who finds your book and opens it to the right page.

## Why an ISO?

An ISO image is a bootable CD/DVD image that contains:

- GRUB bootloader (the first code that runs)
- Our kernel executable
- GRUB configuration (the menu you see when booting)

This is the standard way to distribute and test operating systems. Burn it to a USB drive and it'll boot on real hardware. Or just test it in QEMU—same image works everywhere.

## CMake Targets for ISO Creation

To automate ISO creation, we need to add custom CMake targets to the root `CMakeLists.txt`.

### Prerequisites

First, install the required GRUB tools:

```bash
# Ubuntu/Debian
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools

# macOS
brew install grub xorriso mtools
```

### Adding the Targets

The ISO creation uses a GRUB configuration template. First, create `cmake/grub.cfg.in`:

```
set timeout=0
set default=0

menuentry "TinyOS" {
    multiboot2 /boot/kernel.elf
    boot
}
```

Now open the root `CMakeLists.txt` and add these sections after the `add_subdirectory()` commands:

```diff
 # Memory management
 add_subdirectory(mm)
+
+# Generate grub.cfg from template
+configure_file(
+    ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in
+    ${CMAKE_BINARY_DIR}/grub.cfg
+    @ONLY
+)
+
+# ISO creation target
+add_custom_target(iso
+    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
+    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
+    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
+    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/tinyos.iso ${CMAKE_BINARY_DIR}/iso/
+    DEPENDS kernel.elf ${CMAKE_BINARY_DIR}/grub.cfg
+    COMMENT "Creating bootable ISO image"
+)
+
+# Run in QEMU target
+add_custom_target(run
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso -boot d -serial stdio
+    DEPENDS iso
+    COMMENT "Running TinyOS in QEMU"
+)
+
+# Debug in QEMU target
+add_custom_target(debug
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso -boot d -serial stdio -s -S
+    DEPENDS iso
+    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
+)
 
 # Print build configuration
 message(STATUS "========================================")
```

```

```

**What these targets do:**

- **`iso`** - Creates the ISO directory structure, copies kernel, generates grub.cfg, and creates tinyos.iso
- **`run`** - Depends on `iso`, then boots the ISO in QEMU
- **`debug`** - Same as `run` but with GDB stub enabled on port 1234

Reconfigure CMake to pick up the new targets:

```bash
cmake -B build -G Ninja
```

You'll see output showing the new targets:

```
-- ========================================
-- TinyOS Build Configuration
-- ========================================
-- C Standard: C17
-- Build Type: Debug
-- Compiler: /usr/local/bin/clang
-- Assembler: /usr/bin/nasm
-- ========================================
-- Available targets:
--   ninja           - Build kernel.elf
--   ninja iso       - Create bootable ISO
--   ninja run       - Build and run in QEMU
--   ninja debug     - Build and run with debugger
-- ========================================
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/user/TinyOS/build
```

**What changed?**

1. **GRUB config template** - `cmake/grub.cfg.in` is now version-controlled
2. **CMake generates grub.cfg** - Uses `configure_file()` at configure time
3. **ISO target simplified** - Just copies the generated file instead of echo commands
4. **Dependency tracking** - ISO rebuilds if grub.cfg changes

### Using the Targets

```bash
# Build kernel and create bootable ISO
ninja -C build iso
```

You'll see GRUB and xorriso working to create the bootable image:

```
ninja: Entering directory `build'
[1/1] Creating bootable ISO image
xorriso 1.5.4 : RockRidge filesystem manipulator, libburnia project.

Drive current: -outdev 'stdio:/home/user/TinyOS/build/tinyos.iso'
Media current: stdio file, overwriteable
Media status : is blank
Media summary: 0 sessions, 0 data blocks, 0 data, 80.0g free
Added to ISO image: directory '/'='/tmp/grub.xyz123'
xorriso : UPDATE :     587 files added in 1 seconds
Added to ISO image: directory '/'='/home/user/TinyOS/build/iso'
xorriso : UPDATE :     591 files added in 1 seconds
xorriso : NOTE : Copying to System Area: 512 bytes from file '/usr/lib/grub/i386-pc/boot_hybrid.img'
ISO image produced: 5747 sectors
Written to medium : 5747 sectors at LBA 0
Writing to 'stdio:/home/user/TinyOS/build/tinyos.iso' completed successfully.
```

**What just happened?**

1. **Created directory structure** - `iso/boot/grub/`
2. **Copied kernel** - `kernel.elf` → `iso/boot/kernel.elf`
3. **Generated GRUB config** - `grub.cfg` with our boot menu
4. **Added GRUB modules** - 587 files (GRUB bootloader, modules, fonts)
5. **Added our kernel** - 4 more files (kernel + directory entries)
6. **Created hybrid boot** - Works with both BIOS (i386-pc) and UEFI (x86_64-efi)
7. **Wrote ISO** - Final image is ~11MB (5747 sectors × 2KB)

The result is `build/tinyos.iso` - a bootable ISO image that works on both BIOS and UEFI systems.

### What's in the ISO?

```
build/iso/
├── boot/
│   ├── kernel.elf          ← Our kernel
│   └── grub/
│       └── grub.cfg        ← GRUB configuration
```

The GRUB configuration (`grub.cfg`) is generated from `cmake/grub.cfg.in`:

```
set timeout=0
set default=0

menuentry "TinyOS" {
    multiboot2 /boot/kernel.elf
    boot
}
```

- `multiboot2 /boot/kernel.elf` - Load using Multiboot2 protocol
- `boot` - Start execution

## Testing in QEMU

Time for the moment of truth. Boot the ISO:

```bash
ninja -C build run
```

QEMU should open with... a blank screen. Just black. Nothing.

**Don't panic. This is success.**

I know, I know—a black screen doesn't *feel* like success. But think about what's actually happening behind that void:

1. GRUB boots and scans the first 32KB of your kernel
2. Finds the Multiboot2 magic number (0xe85250d6) ✓
3. Validates the checksum ✓
4. Switches the CPU to 64-bit long mode ✓
5. Loads your kernel at 1MB (0x100000) ✓
6. Jumps to `_start` in your boot assembly ✓
7. Your assembly sets up a stack and calls `kernel_main()` ✓
8. `kernel_main()` checks the Multiboot magic (0x36d76289) ✓
9. Enters the infinite `hlt` loop ✓

**Your code is running on bare metal.** (Well, virtualized bare metal, but still!)

The black screen is expected. We haven't written any code to output text yet. No VGA driver, no serial console—nothing. The kernel is sitting in that `while(1) __asm__("hlt")` loop exactly as designed, waiting patiently for instructions we haven't given it yet.

Press Ctrl+C to exit QEMU. We'll make it more talkative in the next part.

## Debugging with LLDB

Want proof it's actually working? Use LLDB to inspect the running kernel:

```bash
# Terminal 1: Start QEMU with debugger enabled
ninja -C build debug

# Terminal 2: Connect LLDB
lldb build/kernel.elf
(lldb) gdb-remote :1234
(lldb) b kernel_main
(lldb) c
```

When the breakpoint hits at `kernel_main()`, inspect the registers:

```
(lldb) p/x $rdi
(uint64_t) $0 = 0x0000000036d76289  ← Multiboot2 magic number!

(lldb) p/x $rsi
(uint64_t) $1 = 0x0000000000009500  ← Multiboot info structure address

(lldb) n  # Step through magic check - it passes!
(lldb) n  # Step through null check - also passes!
(lldb) n  # Now in the infinite hlt loop
```

You can single-step through your kernel and watch it execute. You've built a real, bootable operating system kernel!

## Common Issues

### ISO Not Created

**Symptom:** `ninja -C build iso` succeeds but no `tinyos.iso` file

**Solution:** Check that you have all required tools:

```bash
which grub-mkrescue xorriso mformat
# All should be found
```

Install missing tools:

```bash
# Arch Linux
sudo pacman -S grub xorriso mtools

# Ubuntu/Debian
sudo apt install grub-pc-bin xorriso mtools
```

### QEMU Exits Immediately

**Symptom:** QEMU starts but immediately exits

**Cause:** The ISO wasn't created properly or GRUB can't find the kernel

**Solution:** Verify the ISO structure:

```bash
ls -R build/iso/
# Should show boot/kernel.elf and boot/grub/grub.cfg
```

### Black Screen Forever

**Symptom:** QEMU shows black screen and hangs

**This is normal!** Your kernel is working correctly. It's sitting in the `hlt` loop. Press Ctrl+C to exit.

## Quick Reference

```bash
ninja -C build         # Build kernel only
ninja -C build iso     # Build kernel and create ISO
ninja -C build run     # Build, create ISO, and boot in QEMU
ninja -C build debug   # Build, create ISO, and boot with debugger
```

## What We've Accomplished

At this point, you have:

- A bootable kernel that loads via GRUB
- Multiboot2 protocol working correctly
- Ability to test changes immediately with `ninja -C build run`
- Debugging setup with LLDB

The kernel doesn't produce any output yet, but it boots and runs correctly. That's a major milestone!

---

**Next: [Summary →](summary.md)**
