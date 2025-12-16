# How to Use This Book

[!side]Typing code yourself is like the difference between watching someone cook and actually cooking. You notice details you'd otherwise miss.[/!side]

I strongly recommend **typing the code yourself** rather than copy-pasting. Muscle memory helps understanding.

That said, if you get stuck or want to skip ahead, the chapter branches are there. No judgment.

The code in this book prioritizes **clarity** over cleverness. We use explicit variable names, comments explaining "why" not "what", consistent formatting, and assertions.

Performance matters in OS development, but premature optimization obscures concepts. We'll optimize only when needed and explain the trade-offs.

[!side]The challenges marked "Advanced" can take hours. They're optional, but that's where the deep learning happens.[/!side]

Most chapters include optional experiments—ways to push the code further or explore alternatives. These aren't required, but breaking things teaches you how they work.

## When Things Go Wrong (And They Will)

Your bootloader won't boot. Your kernel will triple-fault. You'll spend an hour hunting a typo in assembly code. This is *normal*.

Each chapter includes common issues and debugging strategies. When you're truly stuck:

**Take a break.** Walk away. Sleep on it. Fresh eyes catch obvious mistakes.

**Verify your tools.** Tool version mismatches cause strange errors.

**Check the companion code.** Compare against the chapter branch.

**Use the debugger.** LLDB/GDB are essential. We teach debugging in Chapter 3.

**Ask for help.** OSDev forums, Reddit's r/osdev, Discord communities.

Remember: bugs you fix teach you more than code that works on the first try.

---

**Next: [Getting Started](getting-started.md)**
