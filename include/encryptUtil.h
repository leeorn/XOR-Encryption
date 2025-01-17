#ifndef ENCRYPT_UTIL_H
#define ENCRYPT_UTIL_H

/*
 * The number of threads per CPU core (assumption).
 * This constant determines the maximum number of threads that can be created based on the available CPU cores.
 * It is used to calculate the maximum allowed number of threads for the program.
 */
#define THREADS_PER_CORE 4

/*
 * The maximum size of the queue.
 * This constant defines the maximum number of elements that can be stored in the toEncrypt queue.
 */
#define MAX_QUEUE_SIZE 512

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

/*
 * Data structure to hold thread-specific data.
 * This struct contains the data needed by each thread during the encryption process.
 */
typedef struct threadData{
    Queue* toEncrypt;
    Queue* toWrite;
    uint8_t* key;
    long keySize;
    long blockNumber;
    int finishFlag;

} threadData;


/*
 * @brief Reads the encryption key from the specified key file.
 * This function reads the contents of a key file specified by the filename and returns
 * the encryption key as a dynamically allocated uint8_t array.
 * It is the caller's responsibility to free the memory allocated for the key array.
 * 
 * @param [in] filename      - The path to the key file. If it is a relative path, it is relative to the current working directory.
 * @param [out] fileSize     - A pointer to store the size of the key file in bytes.
 * @return A pointer to a array containing the encryption key, or NULL if failure.
*/
uint8_t* readKeyFile(const uint8_t* filename, long* fileSize);


/*
 * @brief Rotates the bits of the encryption key by the specified amount (in place). 
 * It performs a left shift operation on the bits of the encryption key and rolling the LMB to be the RMB.
 *
 * @param [in,out] key  - The unsigned char array of the encryption key to be rotated.
 * @param [in] amount    - The number of times to left-shift the bits of the key.
 * @param [in] size      - The size of the encryption key in bytes.
*/
void rotateKey(uint8_t* key, long amount, long size);


/*
 * @brief Rotates the encryption key left by one bit.
 * The leftmost bit is rolled over to become the rightmost bit.
 *
 * @param [in,out] key  - A char array containing the encryption key.
 * @param [in] size      - The size of the encryption key in bytes.
*/
void leftShiftKey(uint8_t* key, long size);


/*
 * @brief Performs XOR encryption on a block of data using the provided encryption key.
 * 
 * @param [in,out] text     - An unsigned char pointer to the block of data to be encrypted.
 * @param [in] key          - An unsigned char pointer of the encryption key.
 * @param [in] blockSize    - The size of the block to be encrypted.
*/
void encryptBlock(uint8_t* text, const uint8_t* key, long blockSize);


/*
 * @brief Writes the encrypted data to the standard output (stdout).
 * 
 * @param [in] encrypted     - A pointer to the block of encrypted data.
 * @param [in] length        - The size of the encrypted data in bytes.
*/
void writeEncrypted(const uint8_t* encrypted, long legnth);


/*
 * @brief Reads plaintext data, in specified block size, from the standard input (stdin) into an array.
 * 
 * @param [out] input    - A pointer to a block-sized char array to store the input data.
 * @param [in] length    - The size of the input data in bytes.
 * @return The number of bytes actually read.
*/
long readInput(uint8_t* input, long length);


/*
 * @brief The thread function responsible for encrypting data, rotating the key, and writing it to stdout.
 * The function will continue running until both the toEncrypt and toWrite queues are empty and main thread indictes it finished to read from stdin.
 * 
 * @param [in] arg  - A pointer to a threadData structure containing the necessary data for the thread.
 * @return NULL
*/
void* threadFunction(void* arg);

/*
 * @brief Process the input parameters and confirm that the arguments are valid.
 * Searches for the values provided by the user (indicated by '-n' and '-k') for the number of threads and the path to the encryption key file.
 * 
 * @param [in] argc     - The number of command-line arguments.
 * @param [in] argcv    - An array of strings containing the command-line arguments.
 * @param [out] threads - An int pointer variable to store the number of threads requested by the user.
 * @return A pointer to a string containing the path to the encryption key file. Returns NULL if the arguments are invalid.
*/
char* processInput(int argc, char* argv[], int* threads);


#endif