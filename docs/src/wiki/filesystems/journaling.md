# Journaling

Journaling file sytems use a mechanism, called the journal, to keep track of changes that will be made to the file system. 
This allows the file system to recover from crashes and other unexpected events by replaying the journal and applying any changes that were not fully committed to the file system and to keep retrying until the file system is in a consistent state.

For added reliability, some file systems also introduce the database concept of atomic transactions, which group operations together and ensure that either all operations in the transaction are applied to the file system or none of them are. 
This can help prevent data corruption and ensure that the file system remains in a consistent state even in the event of a crash or other unexpected eventoperations together and ensure that either all operations in the transaction are applied to the file system or none of them are. 
This can help prevent data corruption and ensure that the file system remains in a consistent state even in the event of a crash or other unexpected event..
