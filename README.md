# Introduction
This project implements a multithreaded XOR encryption utility in C, designed to process large data sets efficiently. It supports dynamic block sizes based on an external key file, rotating keys for enhanced security, and parallel processing for optimized performance on multi-core systems.

# Instructions
- First, if necessary, unzip the folder.
- Use the 'cd' command to navigate to the unzipped folder.
- The source and header files are located under folder/src and folder/include, respectively.
- The folder/test, which includes mainly different input files (for both key and stdin), is under folder/test.

### Build
I didn't include a Makefile as it appears that the expected commands to run don't utilize it. <br>
To build the program, assuming you're still inside the folder, use : `gcc src/encryptUtil.c src/queue.c -o encryptUtil`.<br>
To run use `cat plaintext | ./encryptUtil -n threadsNum -k keyFile > cyphertext` <br> replace with your desired data. For example, `cat test/input_l.JPG | ./encryptUtil -n 16 -k test/key_s.txt > test/result`.

# Files
### Folders
- `src`: contains the source files (.c)
- `include`: contains the header files (.h)
- `test`: contains test related files, including input data and keys to validate the implementation.
### Files
- `src/encryptUtil.c`: The code to implement XOR stream encryption.
- `src/queue.c`: The code to support the priority queue used by `encryptUtil.c`.
- `include/encryptUtil.h`: The header file for `encryptUtil.c`.
- `include/queue.h`: The header file for `queue.c`.
- `test/X`: Several files that can be used as the input data to be encrypted/decrypted. ('X' is any file there.)
- `README.md`: Explanation file.
- `BTCodingAssignment.pdf`: The assignment details.

# Explanation

At a high level, this project involves encrypting a file using XOR encryption. The process can be broken down into three main tasks: reading the data, processing/encrypting it, and writing it to a destination. Later, the same key can be used to decrypt the encrypted data and recover the original message (any two pieces of information can be used to derive the third one, really.)

The code operates as follows: The main thread starts the program, reads the encryption key, and verifies the provided arguments. It then focuses on reading the input data (from stdin). Each time it reads a block of data (where the block size is equal to the size of the encryption key), the main thread sends it to a queue. This process continues until all the input data has been read. (When the main thread finishes reading, it joins the other thread with the other parts.)

Meanwhile, the other threads (if N > 0) retrieve data from the queue and perform the encryption process. Once encryption is complete, the threads place the encrypted data into another queue, a queue of data waiting for it to be written out. At some point, a thread dequeues the encrypted data and writes it to stdout. The threads determine which/when to write the encrypted data by using the serial number contained within each node's metadata. The queue is implemented as a priority queue (linked list), sorted based on the serial number. The serialization and the priority queue ensure that the blocks are written to stdout in the correct order and improve efficiency during the output process.

To ensure a synchronized pipeline, it is essential to handle potential synchronization issues, particularly when working with queues. The queue implementation includes thread-safe mechanisms. Functions like enqueue, dequeue, get size, etc., take care of acquiring and releasing the locks to maintain thread safety.

Therefore, several tasks occur in parallel: while the main thread reads input and enqueues data, previous data is being encrypted, other data is being enqueued/dequeued, and while data is being written out.<br> 
During testing, I observed that the performance improved significantly when using multiple queues instead of a single queue (N=0) when working with larger files. However, there is a point of diminishing returns when adding more threads, meaning that the improvement in performance becomes less significant. Both observations align with my expectations and make sense to me.

### Notes

- Verified to work on Ubuntu 18.04 and 22.04 with gcc 11.3.0.

- A few design choices were made for this implementation. For example, errors and status messages are written to `stderr` to ensure clean output. Also, for the most part, when an error occurs, the code will exit. Also, the code will not work for cases when the input data is null/empty.

- Overall, the code has undergone extensive testing with various input scenarios to ensure its functionality and reliability. In this case, the code was also tested by encrypting known input data using different keys and verifying the correctness of the output (after the "second pass"). It was tested with many inputs and key sizes, and different types of data to ensure its correctness. Further, each function in the code has been individually tested (unit tests) to ensure its correctness and reliability.

# Future Work
- The current implementation of the priority queue using a linked list may not be optimal for enqueueing a new node. One improvement is to switch to a min heap (array implementation), which can provide better time complexity for enqueue operations.

- In the current implementation, if the main thread has not finished reading the input data but the queue with data waiting to be encrypted becomes large (reaching 75% of its maximum capacity), the main thread sleeps for a short period (0.05 seconds). An alternative approach could be to allow the main thread to assist in clearing the queue, but not until the queue is empty (then others will wait for it.)

- Error handling is currently implemented by printing an error message to stderr and exiting the program. Depending on the desired behavior, an alternative approach can be implemented.
