
#include "../include/queue.h"

Node* createNode(uint8_t* data, long blockSize, long blockNum){
    Node* newNode = (Node*)malloc(sizeof(Node));
    if(newNode == NULL){
        fprintf(stderr, "Error: Failed to allocate memory for the node.\n"); ;
        return NULL;
    }

    if(data == NULL){
        fprintf(stderr, "Error: Can't create a new node. Data is NULL.\n");
        return NULL;
    }
    newNode->data = data;
    newNode->blockSize = blockSize;
    newNode->blockNum = blockNum;
    newNode->next = NULL;

    return newNode;
}


Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if(queue == NULL){
        fprintf(stderr, "Error: Failed to allocate memory for the queue"); 
        return NULL;
    } 

    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->mutexQueue, NULL);

    return queue;
}


int isEmpty(Queue* queue) {
    if(queue == NULL){
        fprintf(stderr, "Error: Given queue is invalid.\n");
        return 0;
    }
    pthread_mutex_lock(&queue->mutexQueue);
    int empty = queue->size == 0;

    pthread_mutex_unlock(&queue->mutexQueue);
    
    return empty;
}


int enqueue(Queue* queue, uint8_t* data, long blockSize, long blockNum){
    pthread_mutex_lock(&queue->mutexQueue);

    Node* newNode = createNode(data, blockSize, blockNum);
    if(newNode == NULL || queue == NULL){
        fprintf(stderr, "Error: Can't enqueue a new node");
        pthread_mutex_unlock(&queue->mutexQueue);
        return 0;
    }

    // Insert to an empty queue
    if(queue->size == 0){
        queue->front = newNode;
        queue->rear = newNode;
        newNode->next = NULL;

        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }

    // If the node should be at the front
    if(blockNum < queue->front->blockNum){
        newNode->next = queue->front;
        queue->front = newNode;

        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }

    // If the node should be at the end
    if(blockNum >= queue->rear->blockNum){
        queue->rear->next = newNode;
        queue->rear = newNode;
        newNode->next = NULL;

        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }
    
    // Insert the node in the middle of the queue
    Node* curr = queue->front;
    while (curr->next != NULL && curr->next->blockNum <= blockNum) {
        curr = curr->next;
    }
    newNode->next = curr->next;
    curr->next = newNode;

    queue->size++;
    pthread_mutex_unlock(&queue->mutexQueue);
    return 1;
}


int enqueueNode(Queue* queue, Node* node){
    if(queue == NULL || node == NULL){
        fprintf(stderr, "Error: Can't enqueue a node.\n");
        return 0;
    }

    pthread_mutex_lock(&queue->mutexQueue);

    // Insert Node in an empty queue
    if(queue->size == 0){
        queue->front = node;
        queue->rear = node;
        node->next = NULL;
        
        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }

    // If the node should be at the front
    if(node->blockNum < queue->front->blockNum){
        node->next = queue->front;
        queue->front = node;

        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }

    // If the node should be at the end
    if(node->blockNum >= queue->rear->blockNum){
        queue->rear->next = node;
        queue->rear = node;
        node->next = NULL;

        queue->size++;
        pthread_mutex_unlock(&queue->mutexQueue);
        return 1;
    }
    
    // Node should be enqueued in the "middle" (between front and rear) of the queue 
    Node* cur = queue->front;
    while(cur->next != NULL && cur->next->blockNum <= node->blockNum){
        cur = cur->next;
    }

    node->next = cur->next;
    cur->next = node;

    queue->size++;
    pthread_mutex_unlock(&queue->mutexQueue);

    return 1;
}


Node* dequeue(Queue* queue) {
    if(queue == NULL){
        fprintf(stderr, "Error: Can't dequeue - Queue is null");
        return NULL;
    }

    pthread_mutex_lock(&queue->mutexQueue);
    Node* frontNode = queue->front;
    
    if(frontNode == NULL){
        pthread_mutex_unlock(&queue->mutexQueue);
        return NULL;
    }

    queue->front = frontNode->next;    
    queue->size--;

    if (queue->size == 0) {
        queue->rear = NULL;
    }

    pthread_mutex_unlock(&queue->mutexQueue);
    return frontNode;
}


int getSize(Queue* queue) {
    pthread_mutex_lock(&queue->mutexQueue);
    long size = queue->size;
    pthread_mutex_unlock(&queue->mutexQueue);
    return size;
}


void queueDistroyMutex(Queue* queue){
    pthread_mutex_destroy(&queue->mutexQueue);
}