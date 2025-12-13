# TinyOS Hardware Abstraction Guide

## Philosophy

> "UNIX is basically a simple operating system, but you have to be a genius to understand the simplicity."
> — *Dennis Ritchie*

**TinyOS provides direct hardware access with minimal abstraction.**

TinyOS embraces low-level hardware programming:

- **Explicit over implicit** - Direct register access with clear intent. No magic.
- **Zero-cost abstractions** - If the abstraction costs performance, it's wrong.
- **Hardware awareness** - Code reflects hardware realities, not programmer fantasies.
- **Type safety** - Strong types prevent register access errors.

Kernel code should directly express hardware operations rather than hiding them behind multiple abstraction layers.

## Hardware Access Patterns

> "Theory and practice sometimes clash. And when that happens, theory loses. Every single time."
> — *Linus Torvalds*

### Memory-Mapped I/O (MMIO)

#### Direct Register Access
Use inline functions with volatile pointers for MMIO:

```c
/**
 * Read from memory-mapped register
 * 
 * @param addr Physical address of register
 * @return Register value
 */
static inline uint32_t mmio_read32(uint64_t addr) {
    return *(uint32_t volatile *) addr;
}

/**
 * Write to memory-mapped register
 * 
 * @param addr Physical address of register
 * @param value Value to write
 */
static inline void mmio_write32(uint64_t addr, uint32_t value) {
    *(uint32_t volatile *) addr = value;
}

// 8-bit variants
static inline uint8_t mmio_read8(uint64_t addr) {
    return *(uint8_t volatile *) addr;
}

static inline void mmio_write8(uint64_t addr, uint8_t value) {
    *(uint8_t volatile *) addr = value;
}

// 64-bit variants
static inline uint64_t mmio_read64(uint64_t addr) {
    return *(uint64_t volatile *) addr;
}

static inline void mmio_write64(uint64_t addr, uint64_t value) {
    *(uint64_t volatile *) addr = value;
}
```

#### Register Structure Definitions
Group related registers in structures:

```c
/**
 * UART register layout
 */
typedef struct {
    uint8_t volatile rbr;      // Receiver Buffer (read) / Transmitter Holding (write)
    uint8_t volatile ier;      // Interrupt Enable
    uint8_t volatile iir;      // Interrupt Identification (read) / FIFO Control (write)
    uint8_t volatile lcr;      // Line Control
    uint8_t volatile mcr;      // Modem Control
    uint8_t volatile lsr;      // Line Status
    uint8_t volatile msr;      // Modem Status
    uint8_t volatile scratch;  // Scratch
} uart_registers_t;

// Access pattern
#define UART0_BASE 0x3F8
uart_registers_t volatile * const uart0 = (uart_registers_t volatile *) UART0_BASE;

void uart_write_byte(uint8_t byte) {
    // Wait for transmitter to be ready
    while ((uart0->lsr & 0x20) == 0) {
        // Busy wait
    }
    uart0->rbr = byte;
}
```

### Port I/O (x86-specific)

#### I/O Port Operations
```c
/**
 * Read byte from I/O port
 * 
 * @param port I/O port number
 * @return Value read from port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * Write byte to I/O port
 * 
 * @param port I/O port number
 * @param value Value to write
 */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Read word (16-bit) from I/O port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * Write word to I/O port
 */
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Read double word (32-bit) from I/O port
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * Write double word to I/O port
 */
static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}
```

#### I/O Delay
For hardware that requires timing delays:

```c
/**
 * Introduce small delay for I/O operations
 * Uses dummy I/O operation to port 0x80
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

// Usage
void slow_hardware_access(void) {
    outb(DEVICE_PORT, command);
    io_wait();  // Give hardware time to respond
    uint8_t status = inb(DEVICE_PORT);
}
```

### Bit Manipulation

#### Bit Operations Macros
```c
// Set specific bit
#define BIT_SET(value, bit)    ((value) |= (1U << (bit)))

// Clear specific bit
#define BIT_CLEAR(value, bit)  ((value) &= ~(1U << (bit)))

// Toggle specific bit
#define BIT_TOGGLE(value, bit) ((value) ^= (1U << (bit)))

// Test if bit is set
#define BIT_TEST(value, bit)   (((value) & (1U << (bit))) != 0)

// Create bitmask
#define BIT_MASK(bit)          (1U << (bit))

// Extract bit field
#define BIT_FIELD_GET(value, shift, mask) (((value) >> (shift)) & (mask))

// Set bit field
#define BIT_FIELD_SET(value, shift, mask, field) \
    ((value) = ((value) & ~((mask) << (shift))) | (((field) & (mask)) << (shift)))
```

#### Usage Examples
```c
// Register bit definitions
#define UART_IER_RDA  0  // Received Data Available
#define UART_IER_THRE 1  // Transmitter Holding Register Empty
#define UART_IER_RLS  2  // Receiver Line Status
#define UART_IER_MS   3  // Modem Status

void uart_enable_interrupts(void) {
    uint8_t ier = 0;
    BIT_SET(ier, UART_IER_RDA);   // Enable receive interrupt
    BIT_SET(ier, UART_IER_THRE);  // Enable transmit interrupt
    uart0->ier = ier;
}

// Multi-bit field example (PIC mask register)
#define IRQ_MASK_TIMER    0
#define IRQ_MASK_KEYBOARD 1
#define IRQ_MASK_CASCADE  2

uint8_t pic_get_mask(void) {
    return inb(0x21);  // Read PIC1 mask register
}

void pic_set_mask(uint8_t mask) {
    outb(0x21, mask);  // Write PIC1 mask register
}

void irq_enable(uint8_t irq_num) {
    uint8_t mask = pic_get_mask();
    BIT_CLEAR(mask, irq_num);  // Clear bit to enable IRQ
    pic_set_mask(mask);
}
```

## Architecture-Specific Code Organization

### Directory Structure
```
arch/
├── x86_64/
│   ├── include/
│   │   ├── cpu.h          # CPU control (CPUID, MSRs)
│   │   ├── io.h           # Port I/O (inb/outb)
│   │   ├── interrupts.h   # IDT, interrupt handling
│   │   └── paging.h       # Page tables, TLB
│   └── src/
│       ├── boot.S         # Early boot assembly
│       ├── cpu.c          # CPU features detection
│       └── interrupts.c   # Interrupt handlers
└── aarch64/              # Future ARM64 support
    └── include/
        └── ...
```

### Conditional Compilation
```c
// arch/common/include/arch.h
#ifndef TINYOS_ARCH_H
#define TINYOS_ARCH_H

#if defined(__x86_64__)
    #include <arch/x86_64/cpu.h>
    #include <arch/x86_64/io.h>
#elif defined(__aarch64__)
    #include <arch/aarch64/cpu.h>
    #include <arch/aarch64/mmio.h>
#else
    #error "Unsupported architecture"
#endif

#endif  // TINYOS_ARCH_H
```

## CPU Control Operations

### x86-64 Specific

#### Control Register Access
```c
/**
 * Read CR0 control register
 */
static inline uint64_t read_cr0(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr0, %0" : "=r"(value));
    return value;
}

/**
 * Write CR0 control register
 */
static inline void write_cr0(uint64_t value) {
    __asm__ volatile("mov %0, %%cr0" : : "r"(value) : "memory");
}

/**
 * Read CR3 (page directory base)
 */
static inline uint64_t read_cr3(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}

/**
 * Write CR3 (load new page directory)
 */
static inline void write_cr3(uint64_t value) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(value) : "memory");
}

/**
 * Read CR4 control register
 */
static inline uint64_t read_cr4(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr4, %0" : "=r"(value));
    return value;
}

/**
 * Write CR4 control register
 */
static inline void write_cr4(uint64_t value) {
    __asm__ volatile("mov %0, %%cr4" : : "r"(value) : "memory");
}
```

#### MSR (Model-Specific Register) Access
```c
/**
 * Read Model-Specific Register
 * 
 * @param msr MSR number
 * @return 64-bit MSR value
 */
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t) high << 32) | low;
}

/**
 * Write Model-Specific Register
 * 
 * @param msr MSR number
 * @param value 64-bit value to write
 */
static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

// Common MSR definitions
#define MSR_EFER         0xC0000080  // Extended Feature Enable Register
#define MSR_STAR         0xC0000081  // System Call Target Address
#define MSR_LSTAR        0xC0000082  // Long Mode System Call Target
#define MSR_KERNEL_GS    0xC0000102  // Kernel GS Base

// Enable syscall/sysret
void enable_syscall(void) {
    uint64_t efer = rdmsr(MSR_EFER);
    efer |= (1 << 0);  // SCE (System Call Extensions)
    wrmsr(MSR_EFER, efer);
}
```

#### CPUID Instruction
```c
/**
 * Execute CPUID instruction
 * 
 * @param leaf CPUID function number
 * @param subleaf CPUID sub-function (for functions that support it)
 * @param eax Output: EAX register
 * @param ebx Output: EBX register
 * @param ecx Output: ECX register
 * @param edx Output: EDX register
 */
static inline void cpuid(
    uint32_t leaf,
    uint32_t subleaf,
    uint32_t * eax,
    uint32_t * ebx,
    uint32_t * ecx,
    uint32_t * edx
) {
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(subleaf));
}

// Check CPU features
int cpu_has_feature_sse2(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 26)) != 0;  // SSE2 is bit 26 of EDX
}
```

### Interrupt Control

#### Interrupt Enable/Disable
```c
/**
 * Disable interrupts
 */
static inline void cli(void) {
    __asm__ volatile("cli" : : : "memory");
}

/**
 * Enable interrupts
 */
static inline void sti(void) {
    __asm__ volatile("sti" : : : "memory");
}

/**
 * Halt CPU until next interrupt
 */
static inline void hlt(void) {
    __asm__ volatile("hlt");
}

/**
 * Read interrupt flag
 * 
 * @return Non-zero if interrupts enabled
 */
static inline int interrupts_enabled(void) {
    uint64_t flags;
    __asm__ volatile("pushfq; pop %0" : "=r"(flags));
    return (flags & (1 << 9)) != 0;  // IF flag is bit 9
}
```

#### Critical Sections
```c
/**
 * Save interrupt state and disable interrupts
 * 
 * @return Previous interrupt state (for restore)
 */
static inline uint64_t irq_save(void) {
    uint64_t flags;
    __asm__ volatile("pushfq; pop %0; cli" : "=r"(flags) : : "memory");
    return flags;
}

/**
 * Restore interrupt state
 * 
 * @param flags Interrupt state from irq_save()
 */
static inline void irq_restore(uint64_t flags) {
    __asm__ volatile("push %0; popfq" : : "r"(flags) : "memory", "cc");
}

// Usage pattern
void critical_operation(void) {
    uint64_t flags = irq_save();
    
    // Critical section - interrupts disabled
    modify_shared_data();
    
    irq_restore(flags);  // Restore previous interrupt state
}
```

## Memory Barriers

### Compiler and Hardware Barriers
```c
/**
 * Compiler barrier
 * Prevents compiler from reordering memory operations
 */
#define barrier() __asm__ volatile("" : : : "memory")

/**
 * Memory fence (x86-64)
 * Full memory barrier - all loads/stores complete
 */
static inline void mfence(void) {
    __asm__ volatile("mfence" : : : "memory");
}

/**
 * Load fence
 * All loads before lfence complete before loads after
 */
static inline void lfence(void) {
    __asm__ volatile("lfence" : : : "memory");
}

/**
 * Store fence
 * All stores before sfence complete before stores after
 */
static inline void sfence(void) {
    __asm__ volatile("sfence" : : : "memory");
}

// Usage with MMIO
void mmio_write_ordered(uint64_t addr1, uint32_t val1, uint64_t addr2, uint32_t val2) {
    mmio_write32(addr1, val1);
    mfence();  // Ensure first write completes
    mmio_write32(addr2, val2);
}
```

## Cache Control

### Cache Line Management
```c
/**
 * Flush cache line containing address
 */
static inline void clflush(void volatile * addr) {
    __asm__ volatile("clflush %0" : : "m"(*(char volatile *) addr));
}

/**
 * Invalidate TLB entry for virtual address
 */
static inline void invlpg(void volatile * addr) {
    __asm__ volatile("invlpg %0" : : "m"(*(char volatile *) addr) : "memory");
}

/**
 * Flush entire TLB
 */
static inline void flush_tlb(void) {
    write_cr3(read_cr3());  // Reload CR3 flushes TLB
}
```

## Atomic Operations

### Lock Prefix Instructions
```c
/**
 * Atomic compare-and-swap
 * 
 * @param ptr Pointer to value
 * @param expected Expected value
 * @param desired Desired new value
 * @return Previous value
 */
static inline uint64_t atomic_cmpxchg(uint64_t volatile * ptr, uint64_t expected, uint64_t desired) {
    uint64_t prev = expected;
    __asm__ volatile("lock cmpxchgq %2, %1"
                     : "=a"(prev), "+m"(*ptr)
                     : "r"(desired), "0"(prev)
                     : "memory");
    return prev;
}

/**
 * Atomic exchange
 */
static inline uint64_t atomic_xchg(uint64_t volatile * ptr, uint64_t new_value) {
    __asm__ volatile("xchgq %0, %1" : "+r"(new_value), "+m"(*ptr) : : "memory");
    return new_value;
}

/**
 * Atomic add and return previous value
 */
static inline uint64_t atomic_fetch_add(uint64_t volatile * ptr, uint64_t value) {
    __asm__ volatile("lock xaddq %0, %1" : "+r"(value), "+m"(*ptr) : : "memory");
    return value;
}

/**
 * Atomic increment
 */
static inline void atomic_inc(uint64_t volatile * ptr) {
    __asm__ volatile("lock incq %0" : "+m"(*ptr) : : "memory");
}
```

## Spinlocks

### Basic Spinlock Implementation
```c
typedef struct {
    uint32_t volatile locked;
} spinlock_t;

#define SPINLOCK_INIT {0}

/**
 * Acquire spinlock
 */
static inline void spinlock_acquire(spinlock_t * lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        while (lock->locked) {
            __asm__ volatile("pause");  // Reduce power consumption
        }
    }
    __sync_synchronize();  // Memory barrier
}

/**
 * Release spinlock
 */
static inline void spinlock_release(spinlock_t * lock) {
    __sync_synchronize();  // Memory barrier
    __sync_lock_release(&lock->locked);
}

/**
 * Try to acquire spinlock without blocking
 * 
 * @return 1 if acquired, 0 if already locked
 */
static inline int spinlock_tryacquire(spinlock_t * lock) {
    if (__sync_lock_test_and_set(&lock->locked, 1) == 0) {
        __sync_synchronize();
        return 1;
    }
    return 0;
}
```

## Hardware Abstraction Layer Organization

### Module Structure
```c
// arch/x86_64/include/io.h - Architecture-specific interface
#ifndef TINYOS_ARCH_X86_64_IO_H
#define TINYOS_ARCH_X86_64_IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
// ... more functions

#endif

// kernel/include/platform_io.h - Platform-independent interface
#ifndef TINYOS_PLATFORM_IO_H
#define TINYOS_PLATFORM_IO_H

#if defined(__x86_64__)
    #include <arch/x86_64/io.h>
    #define platform_io_read8(addr)  inb(addr)
    #define platform_io_write8(addr, val) outb(addr, val)
#elif defined(__aarch64__)
    #include <arch/aarch64/mmio.h>
    #define platform_io_read8(addr)  mmio_read8(addr)
    #define platform_io_write8(addr, val) mmio_write8(addr, val)
#endif

#endif
```

### Device Driver Pattern
```c
// drivers/uart/uart.h - Generic UART interface
typedef struct uart_device uart_device_t;

typedef struct {
    int (*init)(uart_device_t * dev);
    int (*putc)(uart_device_t * dev, char c);
    int (*getc)(uart_device_t * dev);
    int (*configure)(uart_device_t * dev, uint32_t baudrate);
} uart_operations_t;

struct uart_device {
    char const * name;
    uint64_t base_address;
    uart_operations_t const * ops;
    void * private_data;
};

// drivers/uart/8250.c - Specific implementation
int uart_8250_init(uart_device_t * dev) {
    uint64_t base = dev->base_address;
    outb(base + 1, 0x00);  // Disable interrupts
    outb(base + 3, 0x80);  // Enable DLAB
    outb(base + 0, 0x03);  // Set divisor (lo)
    outb(base + 1, 0x00);  // Set divisor (hi)
    outb(base + 3, 0x03);  // 8 bits, no parity, one stop bit
    return 0;
}

uart_operations_t const uart_8250_ops = {
    .init = uart_8250_init,
    .putc = uart_8250_putc,
    .getc = uart_8250_getc,
    .configure = uart_8250_configure
};
```

## Best Practices

1. **Always use volatile for hardware registers**
   - Prevents compiler optimization of MMIO access
   - Ensures every read/write happens as written

2. **Use inline functions over macros when possible**
   - Type safety
   - Better debugging
   - Compiler can optimize equally well

3. **Document hardware quirks**
   - Timing requirements
   - Errata workarounds
   - Implementation-specific behavior

4. **Use memory barriers appropriately**
   - MMIO requires ordering guarantees
   - Atomic operations need synchronization
   - DMA buffers need cache coherency

5. **Test on real hardware**
   - Emulators may not expose all hardware behavior
   - Race conditions appear on multi-core systems
   - Timing-sensitive code behaves differently
