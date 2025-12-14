# TinyOS C Programming Standards

## Language Standard

> "C is not a big language, and it is not well served by a big book."
> — *Dennis Ritchie*

### C17 (ISO/IEC 9899:2018)

TinyOS uses C17 (ISO/IEC 9899:2018) as specified in `.clang-format`:
```yaml
Language: C
Standard: c17
```

This project uses pure C, not C++. See [Philosophy.md](Philosophy.md) for the rationale.

## Compiler Requirements

### Primary Compiler: Clang 17
- Modern error messages and diagnostics
- Cross-compilation support for x86_64-elf target
- LLVM debug information (DWARF-4) for LLDB integration
- Excellent bare-metal support

### Compiler Flags
```cmake
-target x86_64-elf           # Cross-compilation target
-nostdlib                    # No standard library
-ffreestanding               # Freestanding environment
-g3 -gdwarf-4               # Full debug info, DWARF-4 format
-O0                         # No optimization (debug builds)
-Wall -Wextra               # All warnings enabled
-fno-stack-protector        # No stack canaries (kernel manages own security)
-mno-red-zone               # No red zone (required for interrupt handling)
```

## Code Quality Standards

### Warning Policy

> "If the code and the comments disagree, then both are probably wrong."
> — *Norm Schryer*

**Zero warnings. All code must compile cleanly.**

All code must compile without warnings using:
```bash
-Wall -Wextra -Wpedantic
```

**Warnings indicate problems that must be addressed, not suppressed:**
- Unused variables: Remove them
- Implicit function declarations: Add the appropriate include
- Type mismatches: Ensure type correctness
- Signed/unsigned comparisons: Use explicit casts with clear intent
- Missing return statements: Every code path must return a value

### Static Analysis
Code should pass clang static analyzer:
```bash
clang --analyze -Xanalyzer -analyzer-output=text ...
```

### Undefined Behavior

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
- **Userspace**: Process terminates, OS recovers, system continues
- **Kernel**: System crash, all processes terminate, unsaved work lost, potential data corruption

There is no recovery mechanism. There is no debugger popup. The system freezes or reboots.

**Common UB that must be prevented:**
- Uninitialized variables → Always initialize variables
- Buffer overflows → Check array bounds (even one byte overflow causes system crash)
- Null pointer dereferences → Validate pointers before dereferencing
- Signed integer overflow → Use unsigned types or validate ranges
- Precision-losing conversions → Cast explicitly and understand truncation behavior

**This is not theoretical.** Every item in this list can and will crash the entire system if it occurs in kernel code.

## Type System

> "C treats you like a consenting adult. Pascal treats you like a naughty child."
> — *Dennis Ritchie (attributed)*

### Standard Integer Types
Use exact-width types from `<stdint.h>`:
```c
uint8_t   u8;      // 8-bit unsigned
uint16_t  u16;     // 16-bit unsigned
uint32_t  u32;     // 32-bit unsigned
uint64_t  u64;     // 64-bit unsigned

int8_t    i8;      // 8-bit signed
int16_t   i16;     // 16-bit signed
int32_t   i32;     // 32-bit signed
int64_t   i64;     // 64-bit signed

size_t    sz;      // Architecture-dependent size
ptrdiff_t pd;      // Pointer difference
```

### Pointer Types
```c
uintptr_t  uptr;   // Integer large enough to hold a pointer
void*      vptr;   // Generic pointer
```

### Boolean Type
Use standard bool from `<stdbool.h>`:
```c
#include <stdbool.h>

bool is_valid = true;
bool has_error = false;
```

For kernel code where stdbool.h is unavailable:
```c
typedef int bool;
#define true  1
#define false 0
```

### Qualifier Usage
Place qualifiers on the **right** (as per QualifierAlignment: Right):
```c
// Correct
uint32_t const constant_value = 42;
char const * const const_ptr_to_const_data;
uint8_t volatile * hardware_register;

// Not preferred
const uint32_t constant_value = 42;
```

## Memory Management

### No Dynamic Allocation (Initially)
Phase 1 uses only:
- Static allocation (global/static variables)
- Stack allocation (local variables)
- Fixed-size buffers

### Memory Safety
- Always check buffer boundaries
- Initialize all variables before use
- No assumptions about memory contents
- Zero memory before use when security-sensitive

```c
// Good: Explicit initialization
uint8_t buffer[256] = {0};

// Good: Explicit bounds check
if (index < buffer_size) {
    buffer[index] = value;
}

// Bad: Uninitialized
uint8_t buffer[256];  // Contains garbage

// Bad: No bounds check
buffer[index] = value;  // May overflow
```

## Error Handling

### Return Value Convention
- **0 or positive**: Success (may include meaningful value)
- **Negative**: Error code
- **NULL**: Invalid pointer/allocation failure

```c
// Function that can fail
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags) {
    if (!is_valid_address(vaddr)) {
        return -KERNEL_ERROR_INVALID_ADDRESS;
    }
    // ... do work ...
    return 0;  // Success
}

// Pointer-returning function
page_frame_t* mm_alloc_page_frame(void) {
    if (no_free_frames) {
        return NULL;  // Allocation failed
    }
    return frame;
}
```

### Error Checking
Always check return values:
```c
// Good
int result = mm_map_page(vaddr, paddr, flags);
if (result != 0) {
    // Handle error
    return result;
}

// Bad - ignoring return value
mm_map_page(vaddr, paddr, flags);  // What if it fails?
```

## Function Design

### Function Length
- Keep functions focused and short (< 50 lines ideally)
- One function, one purpose
- Extract complex logic into helper functions

### Parameter Order
1. Input parameters (what function reads)
2. Output parameters (what function writes)
3. Size/length parameters

```c
// Good parameter order
void kbuf_copy(
    char_buffer_t const * source,     // Input
    char_buffer_t       * destination, // Output  
    size_t                max_bytes    // Size limit
);
```

### Function Attributes
Use compiler attributes when appropriate:
```c
// Function that never returns
__attribute__((noreturn)) void kernel_panic(const char* message);

// Function that must be called with result checked
__attribute__((warn_unused_result)) int mm_map_page(...);

// Always inline (for performance-critical code)
static inline uint8_t port_read_byte(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
```

## Assembly Integration

> "C is peculiar in a lot of ways, but it, like many other successful things, has a certain unity of approach that stems from development in a small group."
> — *Dennis Ritchie*

### Inline Assembly
Use for hardware-specific operations only:
```c
static inline void arch_enable_interrupts(void) {
    __asm__ volatile ("sti");
}

static inline void arch_disable_interrupts(void) {
    __asm__ volatile ("cli");
}

static inline uint64_t arch_read_cr3(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}
```

### External Assembly
For larger assembly routines, use separate `.asm` files with NASM.

## Platform-Specific Code

### Conditional Compilation
```c
#ifdef __x86_64__
    // x86-64 specific code
    void x86_64_specific_function(void);
#endif

#ifdef __aarch64__
    // ARM64 specific code (future)
    void aarch64_specific_function(void);
#endif
```

### Architecture Abstraction
Provide architecture-agnostic interfaces:
```c
// Generic interface (arch-agnostic)
void arch_enable_interrupts(void);

// x86-64 implementation
#ifdef __x86_64__
static inline void arch_enable_interrupts(void) {
    __asm__ volatile ("sti");
}
#endif
```

## Code Documentation Requirements

### Function Documentation
Every public function requires documentation:
```c
/**
 * Map a virtual page to a physical page frame
 * 
 * Creates a page table entry mapping the virtual address to the
 * physical address with the specified flags.
 * 
 * @param vaddr Virtual address to map (must be page-aligned)
 * @param paddr Physical address of frame (must be page-aligned)
 * @param flags Page flags (present, writable, user, etc.)
 * @return 0 on success, negative error code on failure
 * 
 * @note This function modifies page tables and may flush TLB
 * @warning Caller must ensure addresses are page-aligned
 */
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);
```

### Complex Code Comments
```c
// Explain WHY, not WHAT
// Good:
// Use spinlock here because interrupts may modify this structure
lock_acquire(&frame_allocator_lock);

// Bad:
// Acquire lock
lock_acquire(&frame_allocator_lock);
```

## Preprocessor Usage

### Header Guards
Use include guards in all headers:
```c
#ifndef TINYOS_MM_PAGE_FRAME_H
#define TINYOS_MM_PAGE_FRAME_H

// ... header content ...

#endif  // TINYOS_MM_PAGE_FRAME_H
```

### Macro Safety
Make macros safe with parentheses:
```c
// Good
#define PAGE_ALIGN_DOWN(addr)  ((addr) & ~(PAGE_SIZE - 1))
#define MAX(a, b)              (((a) > (b)) ? (a) : (b))

// Bad (unsafe)
#define PAGE_ALIGN_DOWN(addr)  addr & ~(PAGE_SIZE - 1)  // Broken with expressions
#define MAX(a, b)              a > b ? a : b             // Broken with expressions
```

## Build System Integration

### CMake Integration
Code must work with the modular CMake build system:
- Each module builds as static library
- Clear dependencies declared
- No circular dependencies
- Public headers exported properly

### Cross-Compilation
All code must:
- Work in freestanding environment (no libc)
- Use kernel-provided types and functions
- Respect target architecture constraints (x86_64-elf)