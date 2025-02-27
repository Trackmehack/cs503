Muhammad Farooq
Cs 503
Feb 20 2025
Db homework

1. Yes, I agree that externalizing get_student(...) into its own function is a good design strategy because it makes the code easier to read, reuse, and debug.


2. One reason why the above implementation (returning a pointer to a local variable) would be a very bad idea using the C programming language is that the local variable disappears after the function ends, making the pointer invalid.


3. I think that the alternative implementation of get_student(...) using malloc() would work but if we don’t free the memory, it causes memory leaks.  It works because the memory stays valid after the function ends. 


4.  The reason why the file size reported by the ls command was 128 bytes after adding the student with ID=1, 256 after adding the student with ID=3, and 4160 after adding the student with ID=64 is because the file grows because the student records are stored at their ID positions
  

5. The total storage used on the disk remained unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64 because the disk space stayed the same until ID 64 because it crossed a 4K block limit


 6. When you add a student with a very large student ID (ID=99999) increases the file size to 6400000 as shown by ls but the raw storage only increased to 12K as reported by du is because the file looks big, but Linux only stores actual data, so the real size is small.

