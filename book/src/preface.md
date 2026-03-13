# Preface

> *"What I cannot create, I do not understand."*  
> — Richard Feynman

Operating systems are the unsung heroes of modern computing. They orchastrate computer hardware and software resources to provide the bedrock for all other computer programs that run. Yet despite the ubiquity, they remaon enigmatic for most programmers, used but never truly understood.

Unlike traditional books regarding operating systems, this book isn't a dry textbook that only covers the theoretical aspects. 
It's a hands-on implementation guide that walks you through building a complete operating system from the ground up, one step at a time.

The operating system we will be building in this book is intentionally minimal. 
It won't have all the bells and whistles of a production OS like Linux or Windows. 
Instead, we will focus on the core concepts that make an OS tick.

> TODO: Add other implemented topics from the later chapters, when they are finished

- **Booting**: Getting from power-on to running code
- **Input/Output**: Interfacing with hardware devices

By the end, you'll have a small but complete operating system that boots on real hardware, manages memory, and runs user programs. More importantly, you'll understand *how* and *why* it works.

Each chapter builds incrementally on the last, so it's important to work through the book sequentially.

> TODO: Maybe we will add optional chapters?

All code is available in the accompanying Git repository, with branches for each chapter. If you get stuck or want to skip ahead, you can check out the code for any chapter and keep going.

> **A Personal Note:**
>
> Building an operating system is hard. You'll encounter obscure bugs, cryptic hardware manuals, and moments where nothing makes sense. That's normal. Every OS developer has stared at a blank screen wondering why their kernel won't boot, only to discover a typo in the linker script. 
>
> But the payoff is immense. Few programming experiences match the thrill of watching your kernel boot for the first time, or seeing your memory allocator flawlessly manage gigabytes of RAM."
> The rewards, however, are extraordinary. Little in programming compares to the satisfaction of your first successful kernel boot, or witnessing your memory allocator elegantly manage system resources.
>
>So take your time. Experiment. Break things. 

Let's build an operating system.

---

