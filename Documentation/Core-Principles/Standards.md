# TinyOS C Programming Standards

## Language Standard

### C17 (ISO/IEC 9899:2018)

TinyOS uses C17 (ISO/IEC 9899:2018) as specified in `.clang-format`:

```yaml
Language: C
Standard: c17
```

This project uses pure C, at least for the kernel. Other layers in the OS might use a different language, but C is still the default

## Undefined Behavior in Kernel Space

**Kernel-space undefined behavior has catastrophic consequences.**

In userspace applications, undefined behavior typically crashes the process. The operating system contains the damage through process isolation and memory protection. In kernel space, these protections don't exist.

**Kernel code runs at ring 0 with unrestricted hardware access:**

- No process isolation
- No memory protection between kernel components
- No segmentation faults or exception handlers
- Direct, unmediated hardware access

**Consequences of kernel UB:**

- System crash (triple fault, page fault in interrupt handler, general protection fault)
- Complete hardware lockup requiring power cycle
- Silent memory corruption across the entire system
- Security vulnerabilities with full hardware privileges
- Corrupted boot configuration (damaged boot sector, partition table, or bootloader)
- Data loss (bad pointers can destroy filesystem structures or user data)
- Hardware misconfiguration (incorrect device register programming)

**In userspace vs kernel space:**

- **Userspace**: Process terminates, OS recovers, system continues, not the end of the world
- **Kernel**: System crash, all processes terminate, unsaved work lost, potential data corruption, maybe your disk will get damaged, the firmware might get corrupted, bricking the machine, until you fix the corruption, which might be like looking for a needle in a haystack. In the worst case, you might even permanently damage hardware components. So better be very careful

There is no recovery mechanism. There is no debugger popup. The system freezes or reboots.

**Common UB that must be prevented:**

- Uninitialized variables → Always initialize variables
- Buffer overflows → Check array bounds (even one byte overflow causes system crash)
- Wild, dangling and null pointers
- Integer overflow → Use unsigned types or validate ranges

**This is not theoretical.** Every item in this list can and will crash the entire system if it occurs in kernel code.


