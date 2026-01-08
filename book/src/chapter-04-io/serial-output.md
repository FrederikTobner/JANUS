# Serial Output

Right now our kernel boots, validates the multiboot header and info and then politely sits there doing nothing.
This is emotionally a little bit unfulfilling.

Before we build anything fancy, we need a way to *see* what the kernel is doing.
In user space you might just use simple printf debugging, but in early kernel land the best tool is still the same one it has always been: a serial port.

> **New to serial ports?**
>
> Don’t worry, this isn’t “network programming” in disguise. A UART (Universal Asynchronous Receiver-Transmitter) is basically a tiny byte pipe with a few control knobs.
> On x86, the “knobs” live in *I/O port space*, so we talk to them with `inb`/`outb`.
>
> **The mental model:** COM1 starts at `0x3F8`, and the UART’s registers are just fixed offsets from that base.
> You write characters by placing a byte into the data register. Before you do that, you ask the UART if it’s ready.
>
> **These two instuctions we are going to use in this chapter:**
>
> | Thing | Meaning |
> | ---- | ---- |
> | `outb(port, value)` | Write an 8-bit value to an I/O port |
> | `inb(port)` | Read an 8-bit value from an I/O port |
>
> The rest is just setting the UART into a known mode.
> We use the classic 8N1 framing (8 data bits, no parity, 1 stop bit), enable the FIFO, and then do a loopback test by sending `0xAE` and reading it back.
> If the UART can’t even echo a byte to itself, it’s not going to be a great conversationalist.

[!side]
Yes, it’s 2026 and we’re debugging with a device designed when “plug and play” was mostly a rumour.
Serial is simple, deterministic, and doesn’t care whether your screen driver is currently on fire or like in our case nonexistent.
[/!side]

QEMU makes testing this very easy. If you run QEMU with `-serial stdio`, anything we write to COM1 shows up in the terminal.
Your build already does this for the `run` target, so you don’t need to remember the flags.
Therefor we need to change our `run` and `debug` target to see the serial output.
```cmake-diff
file: CMakeLists.txt
after: DEPENDS iso
---
add_custom_target(run
-    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d ${QEMU_DISPLAY_ARG}
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Running TinyOS in QEMU"
)
```

After that we also need to add it to the `debug` target.

```cmake-diff
file: CMakeLists.txt
after: DEPENDS iso
---
add_custom_target(debug
-    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -s -S ${QEMU_DISPLAY_ARG}
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio -s -S ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
)
```
If you do want to run it manually, this is the idea:

```bash
qemu-system-x86_64 -cdrom ./build/tinyos.iso -boot d -serial stdio
```

That’s the whole trick: give the kernel a UART to talk to, and you get a voice in return.

The PC serial port we’ll use is COM1, whose base I/O port is `0x3F8`. The UART exposes a small bank of registers at fixed offsets from that base.
We’re only going to use the classic “write a byte, wait, write the next byte” path for now. No interrupts. No buffering. No heroics.



We’ll put the public interface in a header and keep the hardware details in a `.c` file.
Your drivers CMake module already picks up all `kernel/drivers/**/*.c` sources automatically, so simply adding the files is enough.

Create the header:

```c-diff
file: kernel/drivers/include/drivers/serial.h
replace: entire file
---
#ifndef DRIVER_SERIAL_H
#define DRIVER_SERIAL_H

#define SERIAL_COM1 0x3F8

// TODO: Add support for COM2, COM3, COM4 and make the baud rate configurable

/**
 * @brief Initialize serial port COM1
 * @return 0 on success, non-zero on failure
 */
int serial_init();

/**
 * @brief Check if the serial transmit buffer is empty
 * @return 1 if empty, 0 otherwise
 */
int serial_is_transmit_empty();

/**
 * @brief Write a character to the serial port
 * @param c Character to write
 */
void serial_write_char(char c);

/**
 * @brief Write a null-terminated string to the serial port
 * @param char_buffer Pointer to the null-terminated character buffer
 *
 * @warning This function expects the character buffer to be null-terminated. Failing to do so will result in UNDEFINED BEHAVIOR
 */
void serial_write_string(char const * char_buffer);

#endif
```

Now the implementation:

```c-diff
file: kernel/drivers/serial/serial.c
replace: entire file
---
#include <drivers/serial.h>
#include <tinyos/types.h>

// Port offsets for COM1
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

static inline void outb(u16 port, u8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int serial_init()
{
    // Disable all interrupts
    outb(SERIAL_COM1 + 1, 0x00);

    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x80);

    // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0x03);
    outb(SERIAL_COM1 + 1, 0x00); // (hi byte)

    // 8 bits, no parity, one stop bit
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1), 0xC7);

    // IRQs enabled, RTS/DSR set
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0B);

    // Set in loopback mode, test the serial chip
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x1E);

    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0xAE);

    // Check if serial is faulty (i.e., not same byte as sent)
    if (inb(SERIAL_DATA_PORT(SERIAL_COM1)) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0F);

    return 0;
}

int serial_is_transmit_empty()
{
    return inb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1)) & 0x20;
}

void serial_write_char(char c)
{
    // Wait for transmit buffer to be empty
    while (!serial_is_transmit_empty())
        ;

    // Send character
    outb(SERIAL_DATA_PORT(SERIAL_COM1), c);
}

void serial_write_string(char const * str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}
```

There are only two moving parts here.
First, `outb`/`inb` talk to the I/O port space (not memory). They are tiny wrappers around the x86 instructions of the same name.
Second, `serial_write_char()` waits until the UART says the transmit buffer is ready, then writes the byte.

[!side]
This is “polling”. It’s not glamorous, but it is hard to mess up.
When we add interrupts later, we can stop busy-waiting and let the CPU do something else.
[/!side]

`serial_init()` is the only mildly spicy part.
It programs the UART for 8N1, turns on the FIFO, and does a quick loopback test (write `0xAE`, read it back). If that test fails, we return a non-zero error code and quietly accept that today is not a serial day.

**Calling it from the kernel.**

With the driver in place, we can initialize it early in `kernel_main()` and print something recognizable.
You don’t need a full logging system yet; a single `serial_write_string()` is enough to prove the pipeline works.

```c-diff
file: kernel/core/main.c
after: #include <boot/multiboot.h>
---
 #include <boot/multiboot.h>
+#include <drivers/serial.h>
```

And then, after your Multiboot checks we will print a "Hello World!" text message to validate that everything is working properly.
Since we worked quite a bit to just print this we will be using ASCII-art here, to reflect the effort we put in.

```c-diff
file: kernel/core/main.c
after: if (info == 0) {
---
     if (info == 0) {
         for (;;) {
             __asm__ volatile("cli; hlt");
         }
     }

+    if (serial_init() == 0) {
+        serial_write_string(
+            " _   _      _ _         _    _            _     _ _ \n"
+            "| | | |    | | |       | |  | |          | |   | | |\n"
+            "| |_| | ___| | | ___   | |  | | ___  _ __| | __| | |\n"
+            "|  _  |/ _ \\ | |/ _ \\  | |/\\| |/ _ \\| '__| |/ _` | |\n"
+            "| | | |  __/ | | (_) | \\  /\\  / (_) | |  | | (_| |_|\n"
+            "\\_| |_/\\___|_|_|\\___/   \\/  \\/ \\___/|_|  |_|\\__,_(_)\n");
+    }
```

Our custom Ninja targets are currently not showing any output from QEMU, only from the build process.
To enable them using the `USE_TERMINAL` keyword.

First we add it to the `run` target.

```cmake-diff
file: CMakeLists.txt
after: DEPENDS iso
---
add_custom_target(run
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio ${QEMU_DISPLAY_ARG}
    DEPENDS iso
+    USES_TERMINAL
    COMMENT "Running TinyOS in QEMU"
)
```

After that we also need to add it to the `debug` target.

```cmake-diff
file: CMakeLists.txt
after: DEPENDS iso
---
add_custom_target(debug
    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio -s -S ${QEMU_DISPLAY_ARG}
    DEPENDS iso
+    USES_TERMINAL
    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
)
```

Now boot the kernel under QEMU. If everything is wired up correctly you should see the output in the terminal from where you ran ninja.

---

**Next: [VGA Text Mode](vga-text-mode.md)**

