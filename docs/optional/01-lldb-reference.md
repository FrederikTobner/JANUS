# LLDB Reference

Since I tend to forget things easily here is an overview of lldb, the command line debugger provided by the llvm project.

## Connecting to QEMU

Launch QEMU with the GDB server enabled using the `debug-limine` build target, then connect LLDB in a second terminal:

```bash
# Terminal 1 — QEMU waits, frozen at the first instruction
cmake --build --preset x86_64-gcc --target debug-limine

# Terminal 2 — attach and set an initial breakpoint
lldb ./build-x86_64-gcc/kernel-limine.elf
(lldb) connect-qemu
(lldb) b kernel_main
(lldb) c
```

The `.lldbinit` at the project root configures the remote protocol and source path mapping automatically when LLDB is launched from the project directory.

## Execution Control

| Command  | Description                            |
|----------|----------------------------------------|
| `c`      | Continue execution                     |
| `n`      | Step over (source line)                |
| `s`      | Step into (source line)                |
| `si`     | Step one machine instruction           |
| `ni`     | Step over one machine instruction      |
| `finish` | Run until the current function returns |

## Breakpoints

```
b kernel_main                        # break on function name
b boot.c:42                          # break at source location
breakpoint set --address 0x100000    # break at physical address
breakpoint list                      # list all breakpoints
breakpoint delete 2                  # delete breakpoint 2
breakpoint enable / disable 1        # toggle without deleting
```

## Inspecting State

```
register read                        # all registers
register read rip rsp rbp            # specific registers (x86_64)
frame variable                       # all locals in the current frame
p my_variable                        # print a variable
p &my_variable                       # print the memory address of a variable
p/x my_variable                      # print in hexadecimal
p *(boot_context_t *)ctx             # dereference a typed pointer
bt                                   # backtrace
bt 10                                # backtrace, limit to 10 frames
f 2                                  # switch to frame 2
```

## Memory Inspection

```
memory read 0xB8000 --count 64       # read 64 bytes from address
x/16xw 0xB8000                       # read 16 words (GDB-style)
memory write 0x1234 -- 0x41          # write a byte
memory find --size 1 0x0 0x100000 0x55  # search physical memory
```

## Disassembly

```
disassemble --frame                  # disassemble around current PC
disassemble --name boot_init         # disassemble a named function
disassemble --start-address 0x100000 --end-address 0x100040
```

## Watchpoints

```
watchpoint set variable my_var       # break on write
watchpoint set variable my_var -w read_write  # break on read or write
watchpoint set expression -- &g_pmm_bitmap    # break on address
watchpoint list
```
