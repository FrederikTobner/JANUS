# VGA Text Mode

The serial port is a great first debugging tool, but it has one awkward downside: it feels like talking to your kernel through a keyhole.
Sometimes you want the machine itself to say something, on its own screen, without needing a terminal window on the side.

On a classic PC there’s a delightfully low-tech way to do this.
VGA text mode isn’t a “driver” in the modern sense. It’s a chunk of memory.
Write characters into it and they appear on screen.

[!side]
If you’ve seen older tutorials talk about setting video modes with `int 0x10`, that’s BIOS territory.
It’s real-mode firmware services.
Our kernel is entered by GRUB in 32-bit protected mode and we switch to long mode immediately after, so BIOS interrupts are not available unless you build a whole real-mode trampoline.
We won’t.
[/!side]

In VGA text mode, the screen is an 80×25 grid.
Each cell is stored as a 16-bit value: the low byte is the character (usually ASCII), the high byte is the attribute (foreground/background color).
The buffer starts at physical address `0xB8000`.

Because our boot code identity maps the first 2 MiB (a single huge page), `0xB8000` is already accessible in long mode without additional paging work.
That’s the whole secret: writing to memory is “I/O”.

We’ll follow the same structure as the serial driver: a small public header under `kernel/drivers/include/drivers/` and the implementation in the drivers tree.
The goal is not to build a terminal emulator.
We just want a reliable place to put bytes.

Create the header:

```c-diff
file: kernel/drivers/include/drivers/vga_text.h
replace: entire file
---
+
+#ifndef DRIVER_VGA_TEXT_H
+#define DRIVER_VGA_TEXT_H
+
+#include <tinyos/types.h>
+
+#define VGA_TEXT_WIDTH 80
+#define VGA_TEXT_HEIGHT 25
+
+void vga_text_init();
+
+void vga_text_clear();
+
+void vga_text_set_color(u8 foreground, u8 background);
+
+void vga_text_putc(char c);
+
+void vga_text_write_string(char const * char_buffer);
+
+#endif
```

And the implementation.
It keeps a cursor (`row`, `column`), a current color attribute, understands `\n` and `\r`, and scrolls when we run out of lines.

```c-diff
file: kernel/drivers/video/vga_text.c
replace: entire file
---
+
+#include <drivers/vga_text.h>
+#include <tinyos/types.h>
+
+#define VGA_TEXT_MEMORY_PHYS 0xB8000
+
+static volatile u16 * const vga_buffer = (volatile u16 *)VGA_TEXT_MEMORY_PHYS;
+
+static u16 vga_row = 0;
+static u16 vga_column = 0;
+static u8 vga_color = 0x07; // light grey on black
+
+static inline u8 vga_entry_color(u8 foreground, u8 background) {
+	const u8 fg = (u8)(foreground & (u8)0x0F);
+	const u8 bg = (u8)((u8)(background & (u8)0x0F) << 4u);
+	return (u8)(fg | bg);
+}
+
+static inline u16 vga_entry(unsigned char uc, u8 color) {
+	return (u16)((u16)uc | (u16)((u16)color << 8u));
+}
+
+static void vga_text_scroll_if_needed() {
+	if (vga_row < VGA_TEXT_HEIGHT) {
+		return;
+	}
+
+	for (u16 y = 1; y < VGA_TEXT_HEIGHT; y++) {
+		for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
+			vga_buffer[(y - 1) * VGA_TEXT_WIDTH + x] = vga_buffer[y * VGA_TEXT_WIDTH + x];
+		}
+	}
+
+	for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
+		vga_buffer[(VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH + x] = vga_entry(' ', vga_color);
+	}
+
+	vga_row = VGA_TEXT_HEIGHT - 1;
+}
+
+void vga_text_init() {
+	vga_row = 0;
+	vga_column = 0;
+	vga_color = vga_entry_color(7, 0);
+	vga_text_clear();
+}
+
+void vga_text_clear() {
+	for (u16 y = 0; y < VGA_TEXT_HEIGHT; y++) {
+		for (u16 x = 0; x < VGA_TEXT_WIDTH; x++) {
+			vga_buffer[y * VGA_TEXT_WIDTH + x] = vga_entry(' ', vga_color);
+		}
+	}
+
+	vga_row = 0;
+	vga_column = 0;
+}
+
+void vga_text_set_color(u8 foreground, u8 background) {
+	vga_color = vga_entry_color(foreground, background);
+}
+
+void vga_text_putc(char c) {
+	if (c == '\n') {
+		vga_column = 0;
+		vga_row++;
+		vga_text_scroll_if_needed();
+		return;
+	}
+
+	if (c == '\r') {
+		vga_column = 0;
+		return;
+	}
+
+	vga_buffer[vga_row * VGA_TEXT_WIDTH + vga_column] = vga_entry((unsigned char)c, vga_color);
+
+	vga_column++;
+	if (vga_column >= VGA_TEXT_WIDTH) {
+		vga_column = 0;
+		vga_row++;
+		vga_text_scroll_if_needed();
+	}
+}
+
+void vga_text_write_string(char const *char_buffer) {
+	for (int i = 0; char_buffer[i] != '\0'; i++) {
+		vga_text_putc(char_buffer[i]);
+	}
+}
```

Now we just need to call it.
This is intentionally unsophisticated: we initialize the driver and print a single line.
The point is to prove the whole pipeline works end-to-end.

```c-diff
file: kernel/core/main.c
after: #include <drivers/serial.h>
---
+#include <drivers/vga_text.h>
```

And then, after the Multiboot sanity checks:

```c-diff
file: kernel/core/main.c
after: checking the multiboot info
---
  if (info == 0) {
    for (;;) {
      __asm__ volatile("cli; hlt");
    }
  }
+  vga_text_init();
+  vga_text_write_string("hello world\n");
```

[!side]
If you notice that the comment block above `kernel_main()` claims we’re in “32-bit protected mode with paging disabled”: that was true earlier in the project.
At the moment, `boot.asm` has already switched us into long mode and enabled paging before it calls into C.
The code is right; the comment will need to catch up.
[/!side]

## TODO

- [ ] Decide whether we want to rely on GRUB’s text mode, or explicitly request a text mode / framebuffer in Multiboot2 and implement the corresponding output path.
- [ ] Add support for backspace, tabs, and basic control characters.
- [ ] Optional: implement the hardware cursor (CRTC ports) and cursor positioning.
- [ ] Consider a small `console` facade that can write to both serial and VGA without forcing printf.
- [ ] Decide how VGA should behave once higher-half mappings are introduced (keep identity mapping, or map a higher-half alias for `0xB8000`).
- [ ] Consider making colors a named enum (instead of raw 0–15 values).
- [ ] Tighten the scroll implementation (memmove-style copy) once you have a trusted `memmove`.

---

**Next: [Keyboard Input](keyboard-input.md)**
