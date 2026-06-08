# Links

There are two types of links in the context of a file system: hard links and symbolic (soft) links.

## Hard links

Hard links are directory entries that point directly to the same underlying file (inode) on the disk. They allow multiple directory entries to refer to the same file, and they share the same inode number. 
When a hard link is created, it increases the reference count of the file, and when a hard link is deleted, it decreases the reference count. 
The file is only deleted from the disk when the reference count reaches zero.

## Symbolic (soft) Links

Symbolic links, also known as soft links, are special files that contain a reference to another file or directory. 
Unlike hard links, symbolic links do not point directly to the underlying file but instead contain a path to the target file.
This makes it possible for symbolic links to point to files on different file systems or even over the network. 
When a symbolic link is accessed, the operating system follows the path contained in the link to access the target file.
This makes them more flexible than hard links, but they can also lead to issues such as broken links if the target file is moved or deleted and additionally worse performance, compared to using hard links, due to the need to resolve the path each time the link is accessed.
