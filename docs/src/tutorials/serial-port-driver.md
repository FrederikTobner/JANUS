# Writing a Serial Port Driver

This tutorial implements a minimal COM1 serial driver for x86_64. Serial output is a kernel developer's first debugging tool — it works before any display driver exists, survives panics, and QEMU can redirect it straight to the host terminal. By the end, `serial_write_string("Hello World!")` will print to the terminal from which QEMU was launched.

## Prerequisites

Familiarity with C and basic x86 concepts (I/O ports, inline assembly). For background on UART hardware, see [Serial Ports](../concepts/hardware/serial-ports.md). The code assumes the kernel is already booting in 64-bit long mode with a working C entry point.

## Port I/O Primitives

x86 has a dedicated I/O port address space, separate from memory. We only need two instructions to access it: `outb` (write a byte) and `inb` (read a byte). We wrap them in a small header so the rest of the driver never touches inline assembly directly.

```c
/* asm/io.h */

#ifndef ASM_IO_H
#define ASM_IO_H

#include <janus/types.h>

static inline void outb(u16 port, u8 value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
```

The GCC inline assembly constraints map directly to what the hardware requires: `"a"` = the `AL` register, `"Nd"` = an 8-bit immediate or the `DX` register. The `volatile` qualifier prevents the compiler from optimising the I/O away.

## UART Register Map

COM1 lives at base address `0x3F8`. The UART exposes a handful of control registers at fixed offsets from that base — here is the register map we care about:

| Offset | Register | Purpose |
| ------ | -------- | ------- |
| +0 | Data | Transmit/receive buffer (or baud divisor low byte when DLAB is set) |
| +1 | Interrupt Enable | (or baud divisor high byte when DLAB is set) |
| +2 | FIFO Control | Enable/configure the 16-byte FIFO |
| +3 | Line Control | Data bits, stop bits, parity, DLAB flag |
| +4 | Modem Control | DTR, RTS, loopback mode |
| +5 | Line Status | Transmit buffer empty, data ready, errors |

Define these as macros so the intent is clear at every call site:

```c
#define SERIAL_COM1 0x3F8

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)
```

## Initialisation

Now we write the initialisation function. `serial_init()` puts the UART into a known state: 38 400 baud, 8N1 framing, FIFO enabled. It finishes with a loopback self-test — if the UART cannot echo `0xAE` back to itself, we know the port is unusable.

```c
int serial_init(void) {
    /* Disable all interrupts */
    outb(SERIAL_COM1 + 1, 0x00);

    /* Enable DLAB (Divisor Latch Access Bit) to set baud rate */
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x80);

    /* Set divisor to 3 → 38 400 baud (115 200 / 3) */
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0x03);   /* low byte  */
    outb(SERIAL_COM1 + 1, 0x00);                  /* high byte */

    /* 8 data bits, no parity, one stop bit (8N1) */
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1), 0x03);

    /* Enable FIFO, clear queues, 14-byte threshold */
    outb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1), 0xC7);

    /* Mark data terminal ready, request to send, enable aux output 2 */
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0B);

    /* Enable loopback mode for self-test */
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x1E);

    /* Send test byte */
    outb(SERIAL_DATA_PORT(SERIAL_COM1), 0xAE);

    /* Check if we get the same byte back */
    if (inb(SERIAL_DATA_PORT(SERIAL_COM1)) != 0xAE) {
        return 1;   /* loopback failed — port unusable */
    }

    /* Switch to normal operation (not loopback) */
    outb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1), 0x0F);

    return 0;
}
```

The DLAB (Divisor Latch Access Bit) is a gate: when set, writes to offset +0 and +1 program the baud divisor instead of touching the data or interrupt registers. After writing the divisor, we clear DLAB by writing `0x03` to the Line Control register, which also configures 8N1.

## Transmitting Data

Before writing a character, we must poll the Line Status register (bit 5) to confirm the transmit buffer is empty. Writing while the buffer is full would silently drop data.

```c
int serial_is_transmit_empty(void) {
    return inb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1)) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_is_transmit_empty())
        ;
    outb(SERIAL_DATA_PORT(SERIAL_COM1), c);
}

void serial_write_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}
```

[!margin]
This is busy-wait polling. It is simple and reliable for early boot output. Once interrupts are available, the driver can be extended to use the UART's interrupt line instead.
[/!margin]

## Using the Driver

Initialise the port early in `kernel_main` and write a test string:

```c
#include <drivers/serial.h>

/* inside kernel_main, after multiboot validation: */
if (serial_init() == 0) {
    serial_write_string("Hello World!\n");
}
```

To see the output, launch QEMU with serial redirected to the host terminal:

```bash
qemu-system-x86_64 -cdrom build/janus.iso -boot d -serial stdio
```

If everything is wired correctly, `Hello World!` appears in the terminal that launched QEMU.

## Key Takeaways

- **Port I/O is not memory I/O.** x86 has a separate address space for devices, accessed with `in`/`out` rather than loads/stores.
- **DLAB multiplexes the register bank.** The same offset serves different purposes depending on whether DLAB is set.
- **Loopback is a cheap self-test.** If the UART cannot echo a byte to itself, everything downstream is pointless.
- **Polling is good enough for early boot.** Interrupt-driven output is an optimisation for later.
