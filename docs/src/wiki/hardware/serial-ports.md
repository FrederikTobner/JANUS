# Serial Ports

A serial port transmits data one bit at a time over a single signal line. In the context of OS development, serial ports serve as the primary debugging and diagnostic channel — they work before any display driver is initialised, they require minimal hardware setup, and their output can be captured by a host machine connected to the other end of the link.

## UART

The hardware that implements serial communication is called a UART (Universal Asynchronous Receiver-Transmitter). "Asynchronous" means the transmitter and receiver do not share a clock signal; instead, both sides agree in advance on a baud rate — the number of signal transitions per second — and each byte is framed with start and stop bits so the receiver can synchronise.

The most common UART on x86 PCs is the 16550, which provides 16-byte transmit and receive FIFOs and is accessed through a set of I/O ports starting at a fixed base address. COM1, the first serial port, is conventionally mapped to I/O port `0x3F8`.

On ARM platforms, the equivalent is the PL011, an AMBA-based UART that is memory-mapped rather than port-mapped. QEMU's virt machine exposes a PL011 at a well-known address, making it the standard serial interface for aarch64 kernel development under emulation.

## Why Serial Matters for Kernels

Serial output is the first thing most kernel developers bring up, for several reasons. It requires no framebuffer, no font renderer, and no display hardware. It works in any CPU mode. It survives panics — because the output goes directly to external hardware, it persists even if the kernel crashes immediately after. And it can be redirected to a log file on the host machine, which makes automated testing and CI straightforward.

Most emulators (QEMU, Bochs) provide virtual serial ports that redirect to the host's terminal or a file, so serial output is accessible even when developing without physical hardware.

## Configuration

A typical bare-metal serial initialisation sequence sets the baud rate divisor, configures the line protocol (typically 8 data bits, no parity, one stop bit — abbreviated 8N1), and enables the transmit FIFO. After that, sending a character is a matter of writing a byte to the transmit holding register once the transmit buffer has space.

[!margin]
See [JANUS serial driver](../../reference/drivers/README.md) for the concrete implementation on both x86_64 and aarch64.
[/!margin]
