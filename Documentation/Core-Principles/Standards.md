# TinyOS C Programming Standards

## Language Standard

> "C is not a big language, and it is not well served by a big book."
> — *Dennis Ritchie*

### C17 (ISO/IEC 9899:2018)

**We use C17. Not C++. Not Rust. C17.**

This is specified in `.clang-format`:
```yaml
Language: C
Standard: c17
```

If you're writing C++ features, you're doing it wrong. Go read Philosophy.md.

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

**Zero warnings. Period.**

All code must compile cleanly with:
```bash
-Wall -Wextra -Wpedantic
```

**Warnings are the compiler telling you that you fucked up.** Fix it. Don't suppress it. Don't ignore it.

Common warnings that mean you fucked up:
- Unused variables (delete them)
- Implicit function declarations (missing include)
- Type mismatches (fix your types)
- Signed/unsigned comparisons (be explicit about what you want)
- Missing return statements (every path must return)

### Static Analysis
Code should pass clang static analyzer:
```bash
clang --analyze -Xanalyzer -analyzer-output=text ...
```

### Undefined Behavior

**In userspace, undefined behavior crashes your application. In kernel space, undefined behavior crashes the entire machine. Or worse - it bricks it.**

You're running in ring 0 with unrestricted hardware access. There are no safety rails. No process isolation. No memory protection. No "your application has stopped responding" dialog. When you fuck up with UB in kernel mode, you get:
- Instant system crash (triple fault, page fault in interrupt handler)
- Complete hardware lockup
- Silent memory corruption across the entire system
- Exploitable security vulnerabilities with full hardware access
- **Permanent hardware damage**: Corrupt the boot sector, partition table, or UEFI firmware and your machine won't boot anymore
- **Actual bricking**: With direct access to firmware flash memory, badly written kernel code can overwrite BIOS/UEFI, turning your motherboard into an expensive paperweight

In a game or text editor, UB means the app crashes and users restart it. Annoying but tolerable. In an OS, a single undefined behavior is a **death sentence**. The entire machine goes down. All running applications die. Unsaved work is gone. (Though if you're a Windows user, you might think weekly BSODs are normal behavior.)

**These are not "implementation details" - they are bugs that will take down the whole system:**
- Uninitialized variables → Initialize your damn variables
- Buffer overflows → Check your bounds (one byte over = kernel panic or worse)
- Null pointer dereferences → Check for NULL (no segfault handler will save you)
- Signed integer overflow → Use unsigned or check limits
- Precision-losing conversions → Cast explicitly and know what you're doing

**The difference between kernel and userspace UB:**
- **Userspace**: Process terminates, OS cleans up, life goes on
- **Kernel**: Triple fault, system reset, angry users, data loss

You don't get exceptions. You don't get debugger popups. You get a frozen screen or instant reboot. Don't write undefined behavior.

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