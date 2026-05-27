# Optimization

Making a file system work efficiently is a complex task, and there are many different techniques that can be used to optimize file system performance. 


## Space Management

In order to keep the file system as efficient as possible, in regards to the amount of space that is used on either the magnetic disk or flash storage, file systems employ various techniques for managing free space and minimizing fragmentation. 
In general there are two main approaches to space management: contiguous allocation and non-contiguous allocation.
Similar to the way how segmentation and paging are used to manage memory, file systems can use contiguous allocation to store files in contiguous blocks on the disk, which can improve performance but can also lead to fragmentation.

### Block size

One of the key aspects to consider when optimizing a file system is the block size, which is the smallest unit of data that can be read or written on the disk.
Choosing an appropriate block size can have a significant impact on file system performance, as it can affect the amount of overhead associated with managing free space and the amount of fragmentation that occurs.
A larger block size can reduce the overhead associated with managing free space, but it can also lead to increased fragmentation, while a smaller block size can reduce fragmentation but can also increase overhead.
Balancing these trade-offs can be considered the key concern when optimizing a file system for performance.

### Tracking free Space

Another important aspect of space management is tracking free space on the disk.
File systems typically use a free space bitmap or a free space list to keep track of which blocks on the disk are free and which are allocated.
The free space list is usually implemented as a linked list of free blocks, while the free space bitmap is a bit array where each bit represents a block on the disk, with a value of 0 indicating that the block is free and a value of 1 indicating that the block is allocated.
The free space bitmap can be more efficient than the free space list, as it allows for faster lookups and can reduce fragmentation, but it can also require more memory to store the bitmap.

When these data structures are allocated using virtual memory, in a file system that supports swapping, the file system can take advantage of the virtual memory system to manage free space more efficiently, by allowing the file system to swap out unused blocks of the free space bitmap or free space list to disk when they are not needed, and swap them back in when they are needed again.
