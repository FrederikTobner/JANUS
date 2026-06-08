# File systems

File systems are a critical component of any operating system, providing a way to organize and manage data on storage devices, like magnetic disks, solid-state drives, or even network storage. 
They provide an abstraction layer to access the data on these devices in an efficient manner.

### Operations

Filesystem suppport the following fundamental operations:
* Tracking the amount of available and used storage space
* Tracking which blocks of data are allocated to which [files](files.md)
* The creation, deletion, and renaming of any existing [files](/docs/src/wiki/filesystems/files.md)

Additionally most filesystems also support:
* Assigning a human-readable name to [files](files.md)
* Rename [files](files.md)
* Allowing files to be divided into non-contigous blocks on the storage device and tracking which blocks belong to which [files](files.md)
* Provoding a hierachrical structure using [directories](directories.md) or folders
* Buffering the result of reading and writing operations, to reduce the amount of operations carried out on the storage device
* Support caching of frequently accessed files or parts of files in memory
* Allowing files to be marked as read-only, to prevent any form of unintentional modification for critical data
* Providing a authorization mechanism to control which users or processes can access or modify files, and what operations they can perform on them

Additionally to these most filesystms also support automatic encryption, employ various [optimization techniques](/docs/src/wiki/filesystems/optimization.md) or [journaling](journaling.md) to improve data integrity and security.

Most filesystems also support the concept of [links](links.md), which allow multiple names to refer to the same file, making the filesystem if it also supports directories a directed acyclic graph (DAG) rather than a tree.

### Indexing Methods

There exist varous methods for indexxing the contents of files, with the most common one being the usage of i-nodes and File allocation tables (FAT). 

### Special types of filesystems

Besides the traditional filesystems, there also exist special types of filesystems, like [flash-based filesystems](/docs/src/wiki/filesystems/flash-based-fs.md) which are optimized for the specific characteristics of flash memory, such as wear leveling and bad block management.

