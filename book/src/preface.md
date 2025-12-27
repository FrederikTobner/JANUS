# Preface

> *"What I cannot create, I do not understand."*  
> — Richard Feynman's blackboard at time of death

Operating systems are the unsung heroes of modern computing. They orchestrate hardware, manage resources, and provide the foundation upon which all software runs. Yet for many programmers, the OS remains a mysterious black box—something you use but never truly understand.

This book is my attempt to demystify that black box.

## Why This Book Exists

I started this project with a simple question: *How does a computer really work?* Not at the level of "click here to run a program," but at the fundamental level where hardware meets software. Where do programs actually live? How does memory really work? What happens in those first microseconds after you press the power button?

The only way to truly answer these questions is to build an operating system yourself.

## What Makes This Book Different

This isn't just a textbook—it's a journal of discovery. Unlike traditional OS textbooks that present finished concepts, this book follows the actual development process of TinyOS. You'll see the mistakes, the iterations, and the "aha!" moments that come from building something complex from scratch.

Every line of code in this book is real, tested, and part of a working system. It's not always the "best" way—sometimes we'll take shortcuts or simplify things for clarity. But it's honest code that boots, runs, and teaches the concepts. You can compile it, run it on real hardware (or QEMU), and modify it to your heart's content.

## A Note on Scope

TinyOS is intentionally minimal. We won't build a full-featured Unix clone or a Windows competitor. Instead, we'll focus on the core concepts that make *any* operating system work:

- **Booting**: Getting from power-on to running code
- **Memory Management**: Controlling the most precious resource
- **Process Scheduling**: Making one CPU appear as many
- **Device Drivers**: Talking to the outside world
- **File Systems**: Persisting data beyond power cycles
- **Input/Output**: Interacting with users and hardware

By the end, you'll have a small but complete operating system that boots on real hardware, manages memory, and runs user programs. More importantly, you'll understand *how* and *why* it works.

## Who This Book Is For

This book assumes you:

- Can read and write C code comfortably
- Understand basic computer architecture (CPU, RAM, storage) and assembly language concepts
- Know how to use a Unix-like command line
- Have patience for low-level debugging

You *don't* need to be an expert in systems programming, assembly language, or hardware. We'll learn those together.

## How to Read This Book

Each chapter builds on the previous one. The code evolves incrementally—we start with a bootloader that prints "Hello" and end with a multitasking kernel. I recommend following along in order, typing outthe code as you go.

All code is available in the accompanying Git repository, with branches for each chapter. If you get stuck or want to skip ahead, you can check out the code for any chapter and keep going.

## A Personal Note

Building an operating system is hard. You'll encounter obscure bugs, cryptic hardware manuals, and moments where nothing makes sense. That's normal. Every OS developer has stared at a blank screen wondering why their kernel won't boot, only to discover a typo in the linker script. 

But the payoff is worth it. Few things in programming are as satisfying as seeing your own kernel boot for the first time, or watching your memory allocator successfully manage gigabytes of RAM. These victories feel earned because they *are* earned.

So take your time. Experiment. Break things. The beauty of working at this level is that when something goes wrong, you have the power to fix it—because you built everything yourself.

Let's build an operating system.

---

