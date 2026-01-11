# Serial Output

Right now our kernel boots, validates the multiboot header and info and then politely sits there doing nothing.
This is emotionally a little bit unfulfilling.

Before we build anything fancy, we need a way to *see* what the kernel is doing.
In user space you might just use simple printf debugging, but in kernel space, the simplest tool is still the same one it has always been: a serial port.

[!side]
Yes, it’s 2026 and we’re debugging with a device designed when “plug and play” was mostly a rumour.
Using the serial port is simple and doesn’t care whether your screen driver is currently on fire or like in our case simply doesn’t exist at this point in time.
[/!side]

> **New to serial ports?**
>
> Don’t worry, this isn’t “network programming” in disguise. A UART (Universal Asynchronous Receiver-Transmitter) is basically a tiny byte pipe with a few control knobs.
> On x86, the “knobs” live in *I/O port space*, so we talk to them with `inb`/`outb`.
>
> COM1 starts at `0x3F8`, and the UART’s registers are just fixed offsets from that base.
> You write characters by placing a byte into the data register. Before you do that, you ask the UART if it’s ready.
>
> **These two instuctions we are going to use in this chapter:**
>
> | Instruction | Meaning |
> | ---- | ---- |
> | `outb(port, value)` | Write an 8-bit value to an I/O port |
> | `inb(port)` | Read an 8-bit value from an I/O port |
>
> The rest is just setting the UART into a known mode.
> We use the classic 8N1 framing (8 data bits, no parity, 1 stop bit), enable the FIFO, and then do a loopback test by sending `0xAE` and reading it back.
> If the UART can’t even echo a byte to itself, it’s not going to be a great conversationalist.

QEMU makes testing this very easy. If you run QEMU with the `-serial stdio` option, anything we write to COM1 shows up in the terminal.
Your build already does this for the `run` target, so you don’t need to remember the flags.
Therefor we need to change our `run` and `debug` target to see the serial output.

```cmake-diff
file: CMakeLists.txt
replace: COMMAND for run target 
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
replace: COMMAND for debug target
---
add_custom_target(debug
-    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -s -S ${QEMU_DISPLAY_ARG}
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos_${TINYOS_TARGET_ARCH}.iso -boot d -serial stdio -s -S ${QEMU_DISPLAY_ARG}
    DEPENDS iso
    COMMENT "Running TinyOS in QEMU with GDB stub"
)
```
If you do want to run it manually, this is the idea:

```bash
qemu-system-x86_64 -cdrom ./build/tinyos.iso -boot d -serial stdio
```

That’s the whole trick: give the kernel a UART to talk to, and you get a voice in return.

The PC serial port we’ll use is COM1, whose base I/O port address is `0x3F8`. The UART exposes a small bank of registers at fixed offsets from that base.
We’re only going to use the classic “write a byte, wait, write the next byte” path for now. No interrupts. No buffering.

We’ll put the public interface in a header and keep the hardware details in a `.c` file.

> TODO: Add CMakeLists.txt entries for the drivers and link with kernel core in the kernel/core/CMakeLists.txt file

```cmake-diff
file: kernel/CMakeLists.txt
after: add_subdirectory(core) 
---
add_subdirectory(boot)
add_subdirectory(core)
+add_subdirectory(drivers)
```

We also need to link the drivers into the kernel core.

```cmake-diff
file: kernel/core/CMakeLists.text
after: LIBRARIES boot
---
tinyos_create_kernel(
    SOURCES
        ${KERNEL_SOURCES}
    LIBRARIES
        boot           
+        drivers      
    LINKER_SCRIPT
        ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)
```

Create the header:

```c-diff
file: kernel/drivers/include/drivers/serial.h
replace: entire file
---
+ #ifndef DRIVER_SERIAL_H
+ #define DRIVER_SERIAL_H
+ 
+ #define SERIAL_COM1 0x3F8
+ 
+ int serial_init();
+ 
+ int serial_is_transmit_empty();
+ 
+ void serial_write_char(char c);
+ 
+ void serial_write_string(char const * char_buffer);
+ 
+ #endif
```

Now the implementation.
First we include the header and the types we have defined for our kernel.

```c-diff
file: kernel/drivers/serial/serial.c
replace: entire file
---
+ #include <drivers/serial.h>
+ #include <tinyos/types.h>
```

Next we define the port offsets for the various UART registers.

```c-diff
file: kernel/drivers/serial/serial.c
after: #include <tinyos/types.h>
---
 #include <drivers/serial.h>
 #include <tinyos/types.h>
 
+ // Port offsets for COM1
+ #define SERIAL_DATA_PORT(base)          (base)
+ #define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
+ #define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
+ #define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
+ #define SERIAL_LINE_STATUS_PORT(base)   (base + 5)
```

Next we need to define the `outb` and `inb` functions to read and write to I/O ports.
These are just thin wrappers around the x86 instructions of the same name.

```c-diff
file: kernel/drivers/serial/serial.c
after: #define SERIAL_LINE_STATUS_PORT(base)   (base + 5)
---
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)
+ 
+ static inline void outb(u16 port, u8 value)
+ {
+     __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
+ }
+ 
+ static inline u8 inb(u16 port)
+ {
+     u8 ret;
+     __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
+     return ret;
+ }
+ 
```

Now that we have the port access functions, we can implement initialization function called `serial_init()`.
It programs the UART for 8N1, turns on the FIFO, and does a quick loopback test (write `0xAE`, read it back). If that test fails, we return a non-zero error code to indicate that the serial port is not usable.

```c-diff
file: kernel/drivers/serial/serial.c
after: inb() function
---
static inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

+ int serial_init()
+ {
+     outb(SERIAL_COM1 + 1, 0x00);
+ 
+     outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x80);
+ 
+     outb(SERIAL_DATA_PORT(SERIAL_COM1), 0x03);
+     outb(SERIAL_COM1 + 1, 0x00); // (hi byte)
+ 
+     outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x03);
+ 
+     outb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1), 0xC7);
+ 
+     outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0B);
+ 
+     outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x1E);
+ 
+     outb(SERIAL_DATA_PORT(SERIAL_COM1), 0xAE);
+ 
+     if (inb(SERIAL_DATA_PORT(SERIAL_COM1)) != 0xAE) {
+         return 1;
+     }
+ 
+     outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0F);
+ 
+     return 0;
+ }
```

Then we will add a function that checks that the current transmit buffer is empty.

```c-diff
file: kernel/drivers/serial/serial.c
after: serial_init() function 
---
int serial_init()
{
    outb(SERIAL_COM1 + 1, 0x00);

    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x80);

    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0x03);
    outb(SERIAL_COM1 + 1, 0x00); // (hi byte)

    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x03);

    outb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1), 0xC7);

    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0B);

    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x1E);

    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0xAE);

    if (inb(SERIAL_DATA_PORT(SERIAL_COM1)) != 0xAE) {
        return 1;
    }

    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0F);

    return 0;
}
 
+ int serial_is_transmit_empty()
+ {
+     return inb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1)) & 0x20;
+ }
+ 
```

Finally we add functions to write a single character and a string to the serial port.

```c-diff
file: kernel/drivers/serial/serial.c
after: serial_is_transmit_empty() function
---
int serial_is_transmit_empty()
{
    return inb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1)) & 0x20;
}

+ void serial_write_char(char c)
+ {
+     while (!serial_is_transmit_empty())
+         ;
+ 
+     outb(SERIAL_DATA_PORT(SERIAL_COM1), c);
+ }
+ 
+ void serial_write_string(char const * str)
+ {
+     for (int i = 0; str[i] != '\0'; i++) {
+         serial_write_char(str[i]);
+     }
+ }
```


There are only two moving parts here.
First, `outb`/`inb` talk to the I/O port space (not memory). They are tiny wrappers around the x86 instructions of the same name.
Second, `serial_write_char()` waits until the UART says the transmit buffer is ready, then writes the byte.

[!side]
This is “polling”. It’s not glamorous, but it is hard to mess up.
When we add interrupts later, we can stop busy-waiting and let the CPU do something else.
[/!side]

`serial_init()` is the only mildly spicy part.


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
+        serial_write_string("Hello World!");
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

