#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>


/*
 * Node data representing a node in the queue.
 * This struct contains a data block to be processed.
 */
typedef struct Node {
    uint8_t* data;
    long blockSize;
    long blockNum;
    struct Node* next;
} Node;


/*
 * Data structure representing a queue.
 * This struct encapsulates the necessary information to manage the queues.
 */
typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
    pthread_mutex_t mutexQueue;
} Queue;


/*
 * @brief Create a new node to store relevant information.
 * The function creates a new node and initializes its data members.
 * The node is used to store a block of data with its associated metadata.
 * The caller is responsible for freeing the allocated memory.
 * 
 * @param [in] data         - An unsigned char array to store data. It will have a length equal to blockSize.
 * @param [out] blockSize   - The size of the current block.
 * @param [out] blockNum    - The serial number of the current block. If this block was the Nth block to be read, it will be the Nth block to be written.
 * @return A pointer to the created node if successful. Otherwise, returns NULL.
*/
Node* createNode(uint8_t* data, long blockSize, long blockNum);


/*
 * @brief Create an empty queue.
 * This function creates a new empty queue and initializes its data members.
 * The queue is intended to be a priority queue based on the Nodes' blockNumber in ascending order.
 * The caller is responsible for freeing the allocated memory.
 * 
 * @return A pointer to the created queue if successful. Otherwise, returns NULL.
*/
Queue* createQueue();


/*
 * @brief Check if the queue is empty.
 * The function is thread safe and acquires the lock to ensure synchronized access.
 * 
 * @param [in] queue - A pointer to the queue data structure.
 * @return Return 1 if the queue is empty, else 0.
*/
int isEmpty(Queue* queue);


/*
 * @brief Enqueue a node into the given queue. 
 * The function will create a new node and add it to the queue in its appropriate location based on the blockNum value. 
 * The function is thread-safe and acquires the lock to ensure synchronized access.
 * 
 * @param [in] queue        - A pointer to the queue data structure.
 * @param [in] data         - An unsigned char data.
 * @param [in] blockSize    - The size of the data block.
 * @param [in] blockNum     - The serial number of the data block.
 * @return Return 1 if successful insertion, else 0.
*/
int enqueue(Queue* queue, uint8_t* data, long blockSize, long blockNum);


/*
 * @brief Enqueue a node into the given queue. 
 * The function will add an existing node to the queue in its appropriate location based on the blockNum value.
 * It's useful when we just want to move a node from one queue to another.
 * The function is thread-safe and acquires the lock to ensure synchronized access.
 * 
 * @param [in] queue    - A pointer to the queue data structure.
 * @param [in] node     - A pointer to a node.
 * @return Return 1 if successful insertion, else 0.
*/
int enqueueNode(Queue* queue, Node* node);


/*
 * @brief Dequeue a node from the given queue. 
 * The function detaches the first node in the queue and returns it.
 * The function is thread-safe and acquires the lock to ensure synchronized access.
 * 
 * @param [in] queue    - A pointer to the queue data structure.
 * @return Return the front node if successful. NULL else.
*/
Node* dequeue(Queue* queue);


/*
 * @brief Get the queue size.
 * The function returns the number of nodes currently in the queue. 
 * The function is thread-safe and acquires the lock to ensure synchronized access.
 * 
 * @param [in] queue    - A pointer to the queue data structure.
 * @return Return queue's size.
*/
int getSize(Queue* queue);


/*
 * @brief Close the mutex of the queue.
 * The function destroys the mutex associated with the queue.
 * 
 * @param [in] queue    - A pointer to the queue data structure.
*/
void queueDistroyMutex(Queue* queue);


#endif 