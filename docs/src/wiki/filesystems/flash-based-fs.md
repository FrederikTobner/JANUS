# Flash-based filesystems

The nowadys most commonly known filesystems have been developed for use on magnetic storage media, and their design reflects the performance characteristics of those devices. Flash-based storage, such as SSDs and eMMC, has different performance profiles and failure modes, which has led to the development of flash-optimized filesystems.

## Organization

Flash based filesystems are typically organized around the concept of flash pages and blocks. A flash page is the smallest unit of data that can be read or written, while a flash block is a larger unit that must be erased before it can be rewritten. This organization allows flash-based filesystems to optimize for the unique characteristics of flash storage, such as wear leveling and garbage collection.

### Flash pages

A flash page is the smallest unit of data that can be read or written on a flash storage device. Flash-based filesystems are designed to optimize for the performance characteristics of flash pages, which typically have fast read and write speeds but limited endurance. To minimize wear on the flash storage, flash-based filesystems often use techniques such as wear leveling, which distributes writes evenly across the storage device to prevent any single page from wearing out prematurely.

### Flash blocks

A flash block is a larger unit of data that must be erased before it can be rewritten on a flash storage device. Flash-based filesystems are designed to optimize for the performance characteristics of flash blocks, which typically have slower erase times than read and write times. To minimize the impact of slow erase times, flash-based filesystems often use techniques such as garbage collection, which identifies and consolidates free space on the storage device to reduce the number of erase operations needed.

Popular examples of flash-based filesystems include:
* F2FS (Flash-Friendly File System) - developed by Samsung for use on NAND flash storage, designed to minimize write amplification and improve performance, supported by the linux kernel.
