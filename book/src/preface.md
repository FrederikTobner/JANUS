# Preface

> *"What I cannot create, I do not understand."*  
> — Richard Feynman

Operating systems are the unsung heroes of modern computing. They manage computer hardware and software resources, and provide the foundation upon which all other computer programs run. runs. Yet for most programmers, the OS remains a mysterious black box—something you use but never truly understand.

Unlike traditional books regarding operating systems, this book isn't a dry textbook that only covers the theoretical aspects. 
It's an implementation guide that walks you through building a simple operating system from scratch.
We'll build a simple, yet complete operating system from the ground up, step by step.

The operating system we will be building in this book is intentionally minimal. It won't have all the bells and whistles of a production OS like Linux or Windows. Instead, we will focus on the core concepts that make an OS tick:

> TODO: Add other implemented topics from the later chapters, when they are finished

- **Booting**: Getting from power-on to running code

By the end, you'll have a small but complete operating system that boots on real hardware, manages memory, and runs user programs. More importantly, you'll understand *how* and *why* it works.

Each chapter builds on the previous one. The code evolves incrementally, therefor I it is necessary to follow along from the beginning.

> TODO: Maybe we will add optional chapters?

All code is available in the accompanying Git repository, with branches for each chapter. If you get stuck or want to skip ahead, you can check out the code for any chapter and keep going.

> **A Personal Note:**
>
> Building an operating system is hard. You'll encounter obscure bugs, cryptic hardware manuals, and moments where nothing makes sense. That's normal. Every OS developer has stared at a blank screen wondering why their kernel won't boot, only to discover a typo in the linker script. 
>
> But the payoff will be worth it. Few things in programming are as satisfying as seeing your own kernel boot for the first time, or watching your memory allocator successfully manage gigabytes of RAM. These victories feel earned because they *are* earned.
>
>So take your time. Experiment. Break things. The beauty of working at this level is that when something goes wrong, you have the power to fix it—because you built everything yourself.

Let's build an operating system.

---

