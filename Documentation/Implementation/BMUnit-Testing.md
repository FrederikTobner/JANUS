# BMUnit: Bare Metal Unit Testing Framework

## Overview

**BMUnit** (Bare Metal Unit) is TinyOS's in-kernel testing framework, inspired by Linux kernel's KUnit but adapted for bare metal x86-64 development.

**Design Philosophy:**
- **Pure C17** - No C++ bullshit
- **Freestanding** - Runs in kernel context, no libc
- **Embedded tests** - Tests live with the code they verify
- **Minimal overhead** - Simple macros, no runtime bloat
- **Conditional compilation** - Tests compiled only when `BUILD_TESTS` is enabled

## Why BMUnit, Not GoogleTest?

> "Simple things should be simple, complex things should be possible."
> — *Alan Kay*

GoogleTest is C++. We use C. End of discussion.

Beyond the language issue:
- GoogleTest requires exceptions, RTTI, and STL
- Needs full userspace environment with heap allocation
- Assumes threading primitives (pthreads)
- Object-oriented testing model doesn't fit kernel code

**BMUnit is designed for kernel development. GoogleTest is not.**

## Why BMUnit, Not KUnit Directly?

BMUnit is **based on KUnit** but adapted for TinyOS's specific needs:

**Similarities to KUnit:**
- Same basic assertion macros (`BMUNIT_EXPECT_EQ`, `BMUNIT_ASSERT_TRUE`, etc.)
- Test case and test suite structure
- Embedded in modules with conditional compilation

**Differences from KUnit:**
- **Simpler initialization** - KUnit assumes full Linux kernel environment
- **Custom output** - KUnit uses `printk`, we use our serial/VGA output
- **Bare metal focus** - No assumptions about kernel subsystems being available
- **Extensible** - Add features as TinyOS needs them, not what Linux needs

**BMUnit may add features KUnit doesn't have or omit features we don't need.** It's our framework, designed for our kernel.

## Testing Approach: Embedded Tests

> "Testing shows the presence, not the absence of bugs."
> — *Edsger Dijkstra*

### Tests Live WITH Code, Not Separately

**Like Linux kernel, tests are embedded in the modules they verify:**

```
lib/libkbuffer/
├── buffer.c              # Implementation
├── buffer_test.c         # Tests (compiled with BUILD_TESTS)
├── include/
│   └── lib/
│       └── buffer.h
└── CMakeLists.txt

arch/x86_64/
├── io.c
├── io_test.c             # Tests for I/O operations
├── serial.c
├── serial_test.c
└── CMakeLists.txt

kernel/
├── main.c
├── panic.c
├── panic_test.c          # Panic handling tests
└── CMakeLists.txt
```

**Why?**
- **Locality** - Tests right next to the code they test
- **Ownership** - Module maintainer owns both code and tests
- **No artificial separation** - Tests are part of the module
- **Kernel standard** - How Linux does it

### No Separate `tests/` Folder

We don't use a separate top-level `tests/` directory. That's a userspace convention that doesn't fit kernel development.

## BMUnit API

> "If you need more than 3 levels of indentation, you're screwed anyway, and should fix your program."
> — *Linus Torvalds*

### Basic Test Structure

```c
// lib/libkbuffer/buffer_test.c
#include <bmunit/test.h>
#include <lib/buffer.h>

static void test_buffer_create(struct bmunit *test)
{
    char_buffer_t * buf = charbuf_create(64);
    
    BMUNIT_EXPECT_NOT_NULL(test, buf);
    BMUNIT_EXPECT_EQ(test, buf->capacity, 64);
    BMUNIT_EXPECT_EQ(test, buf->length, 0);
    
    charbuf_destroy(buf);
}

static void test_buffer_append(struct bmunit *test)
{
    char_buffer_t * buf = charbuf_create(16);
    
    int result = charbuf_append(buf, "Hello", 5);
    BMUNIT_EXPECT_EQ(test, result, 0);
    BMUNIT_EXPECT_EQ(test, buf->length, 5);
    
    charbuf_destroy(buf);
}

static struct bmunit_case buffer_test_cases[] = {
    BMUNIT_CASE(test_buffer_create),
    BMUNIT_CASE(test_buffer_append),
    {}
};

static struct bmunit_suite buffer_test_suite = {
    .name = "charbuffer",
    .test_cases = buffer_test_cases,
};

bmunit_test_suite(buffer_test_suite);
```

### Assertion Macros

#### Equality Assertions
```c
BMUNIT_EXPECT_EQ(test, actual, expected)      // Continue on failure
BMUNIT_EXPECT_NE(test, actual, expected)      // Not equal
BMUNIT_ASSERT_EQ(test, actual, expected)      // Abort test on failure
BMUNIT_ASSERT_NE(test, actual, expected)
```

#### Boolean Assertions
```c
BMUNIT_EXPECT_TRUE(test, condition)
BMUNIT_EXPECT_FALSE(test, condition)
BMUNIT_ASSERT_TRUE(test, condition)
BMUNIT_ASSERT_FALSE(test, condition)
```

#### Pointer Assertions
```c
BMUNIT_EXPECT_NULL(test, ptr)
BMUNIT_EXPECT_NOT_NULL(test, ptr)
BMUNIT_ASSERT_NULL(test, ptr)
BMUNIT_ASSERT_NOT_NULL(test, ptr)
```

#### Pointer Equality
```c
BMUNIT_EXPECT_PTR_EQ(test, ptr1, ptr2)        // Same address
BMUNIT_EXPECT_PTR_NE(test, ptr1, ptr2)        // Different address
```

#### String Assertions (when string functions available)
```c
BMUNIT_EXPECT_STREQ(test, str1, str2)         // Strings equal
BMUNIT_EXPECT_STRNE(test, str1, str2)         // Strings not equal
```

### EXPECT vs ASSERT

**`BMUNIT_EXPECT_*`** - Test continues even if assertion fails (checks multiple conditions)

**`BMUNIT_ASSERT_*`** - Test aborts immediately on failure (preconditions, setup checks)

```c
static void test_example(struct bmunit *test)
{
    char_buffer_t * buf = charbuf_create(64);
    
    // ASSERT: If this fails, no point continuing
    BMUNIT_ASSERT_NOT_NULL(test, buf);
    
    // EXPECT: Check multiple properties even if one fails
    BMUNIT_EXPECT_EQ(test, buf->capacity, 64);
    BMUNIT_EXPECT_EQ(test, buf->length, 0);
    BMUNIT_EXPECT_NOT_NULL(test, buf->data);
    
    charbuf_destroy(buf);
}
```

### Test Suite Definition

```c
// Define test cases
static struct bmunit_case my_test_cases[] = {
    BMUNIT_CASE(test_function_1),
    BMUNIT_CASE(test_function_2),
    BMUNIT_CASE(test_function_3),
    {}  // Terminator
};

// Define test suite
static struct bmunit_suite my_test_suite = {
    .name = "module_name",
    .test_cases = my_test_cases,
};

// Register suite (runs during kernel init if BUILD_TESTS enabled)
bmunit_test_suite(my_test_suite);
```

### Test Fixtures (Setup/Teardown)

For tests that need common setup:

```c
static int buffer_suite_init(struct bmunit_suite *suite)
{
    // Suite-level initialization
    // Called once before any test in suite runs
    return 0;  // 0 = success
}

static void buffer_suite_exit(struct bmunit_suite *suite)
{
    // Suite-level cleanup
    // Called once after all tests complete
}

static int buffer_test_init(struct bmunit *test)
{
    // Per-test initialization
    // Called before each test case
    return 0;
}

static void buffer_test_exit(struct bmunit *test)
{
    // Per-test cleanup
    // Called after each test case
}

static struct bmunit_suite buffer_test_suite = {
    .name = "charbuffer",
    .init = buffer_suite_init,
    .exit = buffer_suite_exit,
    .test_cases = buffer_test_cases,
};
```

## CMake Integration

### Module CMakeLists.txt

```cmake
# lib/libkbuffer/CMakeLists.txt

# Always build the library
add_library(kbuffer STATIC
    buffer.c
)

target_include_directories(kbuffer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Conditionally build tests
if(BUILD_TESTS)
    add_library(kbuffer_test STATIC
        buffer_test.c
    )
    
    target_link_libraries(kbuffer_test
        kbuffer
        bmunit
    )
    
    # Register test suite
    set_property(GLOBAL APPEND PROPERTY BMUNIT_TEST_MODULES kbuffer_test)
endif()
```

### Root CMakeLists.txt

```cmake
# Enable testing support
option(BUILD_TESTS "Build BMUnit tests" OFF)

if(BUILD_TESTS)
    message(STATUS "BMUnit: Testing enabled")
    add_subdirectory(bmunit)  # BMUnit framework
endif()

# ... rest of build ...

if(BUILD_TESTS)
    # Collect all test modules
    get_property(TEST_MODULES GLOBAL PROPERTY BMUNIT_TEST_MODULES)
    
    # Create test kernel target that includes all tests
    add_executable(tinyos_test.elf
        ${KERNEL_SOURCES}
        ${BOOT_SOURCES}
    )
    
    target_link_libraries(tinyos_test.elf
        ${TEST_MODULES}
        bmunit
        # ... other libraries ...
    )
endif()
```

### Building and Running Tests

```bash
# Configure with tests enabled
cmake -B build -DBUILD_TESTS=ON

# Build test kernel
cmake --build build --target tinyos_test.elf

# Run in QEMU with test output
qemu-system-x86_64 -kernel build/tinyos_test.elf -serial stdio -display none
```

## BMUnit Framework Implementation

### Directory Structure

```
bmunit/
├── include/
│   └── bmunit/
│       ├── test.h           # Main BMUnit API
│       ├── types.h          # BMUnit types
│       └── runner.h         # Test runner
├── bmunit_core.c            # Core framework
├── bmunit_assertions.c      # Assertion implementations
├── bmunit_runner.c          # Test execution
└── CMakeLists.txt
```

### Framework Types

```c
// include/bmunit/types.h

struct bmunit;
struct bmunit_suite;

typedef void (*bmunit_test_func_t)(struct bmunit *test);
typedef int (*bmunit_init_func_t)(struct bmunit_suite *suite);
typedef void (*bmunit_exit_func_t)(struct bmunit_suite *suite);

struct bmunit_case {
    char const * name;
    bmunit_test_func_t run;
};

struct bmunit_suite {
    char const * name;
    bmunit_init_func_t init;
    bmunit_exit_func_t exit;
    struct bmunit_case const * test_cases;
};

struct bmunit {
    struct bmunit_suite const * suite;
    char const * test_name;
    bool failed;
    uint32_t assertions_passed;
    uint32_t assertions_failed;
};
```

## Test Output

BMUnit provides formatted test output via serial/VGA:

```
[BMUnit] Running test suite: charbuffer
  [1/3] test_buffer_create...................... PASS
  [2/3] test_buffer_append...................... PASS
  [3/3] test_buffer_destroy.................... PASS
  
  Suite: charbuffer - 3 tests, 3 passed, 0 failed

[BMUnit] Running test suite: serial
  [1/2] test_serial_init........................ PASS
  [2/2] test_serial_write....................... FAIL
    Expected: 0
    Actual:   -1
    Location: arch/x86_64/serial_test.c:42
  
  Suite: serial - 2 tests, 1 passed, 1 failed

==============================================
BMUnit Summary: 5 tests, 4 passed, 1 failed
==============================================
```

## Testing Best Practices

> "Regression testing? What's that? If it compiles, it is good; if it boots up, it is perfect."
> — *Linus Torvalds (sarcastically)*

### Test Organization

```c
// One test file per implementation file
// buffer.c -> buffer_test.c
// serial.c -> serial_test.c
```

### Test Naming

```c
// test_<module>_<function>_<scenario>
static void test_buffer_append_empty_buffer(struct bmunit *test) { }
static void test_buffer_append_full_buffer(struct bmunit *test) { }
static void test_serial_init_default_config(struct bmunit *test) { }
```

### What to Test

**Test public APIs:**
- Input validation (null pointers, invalid values)
- Expected behavior with valid inputs
- Boundary conditions (empty, full, maximum size)
- Error handling (return codes, error states)

**Don't test:**
- Internal helper functions (test through public API)
- Trivial getters/setters unless they have logic
- Code that just wraps hardware (test integration instead)

### Test Independence

Each test should be independent:

```c
// GOOD: Self-contained
static void test_buffer_append(struct bmunit *test)
{
    char_buffer_t * buf = charbuf_create(64);  // Local setup
    charbuf_append(buf, "test", 4);
    BMUNIT_EXPECT_EQ(test, buf->length, 4);
    charbuf_destroy(buf);                       // Local cleanup
}

// BAD: Depends on global state
static char_buffer_t * global_buf;  // Shared state

static void test_setup(struct bmunit *test) {
    global_buf = charbuf_create(64);
}

static void test_append(struct bmunit *test) {
    charbuf_append(global_buf, "test", 4);  // Depends on test_setup
}
```

## Advanced Features (Future)

BMUnit may add features beyond KUnit as TinyOS needs them:

### Parameterized Tests (Planned)
```c
// Run same test with different inputs
BMUNIT_PARAMETERIZED_TEST(test_buffer_sizes, size, 
    16, 32, 64, 128, 256
) {
    char_buffer_t * buf = charbuf_create(size);
    BMUNIT_EXPECT_EQ(test, buf->capacity, size);
    charbuf_destroy(buf);
}
```

### Performance Benchmarks (Planned)
```c
BMUNIT_BENCHMARK(bench_buffer_append) {
    char_buffer_t * buf = charbuf_create(1024);
    
    BMUNIT_BENCHMARK_START(test);
    for (int i = 0; i < 1000; i++) {
        charbuf_append(buf, "x", 1);
    }
    BMUNIT_BENCHMARK_END(test);
    
    charbuf_destroy(buf);
}
```

### Hardware Mock Support (Planned)
```c
// Mock hardware registers for testing without real hardware
BMUNIT_MOCK_MMIO(UART_BASE, uart_mock_data);
test_serial_write();  // Uses mock instead of real UART
```

## Differences from KUnit

| Feature | KUnit | BMUnit |
|---------|-------|--------|
| Output | `printk` | Serial/VGA via `kio_*` |
| Initialization | Linux subsystem init | Bare metal early init |
| Memory allocation | `kmalloc` | Static/stack (Phase 1) |
| String operations | Linux `<string.h>` | `libkstd` |
| Test discovery | Module loading | Linker section gathering |
| Parameterized tests | Yes | Planned |
| Hardware mocking | No | Planned |

**BMUnit takes what works from KUnit and adapts it for bare metal kernel development.**

## References

- Linux KUnit documentation: https://www.kernel.org/doc/html/latest/dev-tools/kunit/
- Linux kernel testing: https://www.kernel.org/doc/html/latest/dev-tools/testing-overview.html
