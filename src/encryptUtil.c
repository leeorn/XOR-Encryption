#include "../include/encryptUtil.h"

pthread_mutex_t mutexBlockNum;

uint8_t* readKeyFile(const uint8_t* filename, long* fileSize){
    FILE* file = fopen(filename, "rb");
    // Try to read the file, indicates if faild
    if(file == NULL){
        fprintf(stderr, "Error: opening the file!\n");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    if(*fileSize == -1){
        fprintf(stderr, "Error: getting file size!\n");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the key file
    uint8_t* array = (uint8_t*)malloc(*fileSize);
    if(array == NULL){
        fprintf(stderr, "Error: allocating memory!\n");
        fclose(file);
        return NULL;
    }

    // Read the key file
    size_t fileBytes = fread(array, 1, *fileSize, file);
    if(fileBytes != *fileSize){
        fprintf(stderr, "Error: reading the file!\n");
        free(array);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return array;
}


void rotateKey(uint8_t* key, long amount, long size){
    for(long i = 0; i < amount; i++){
        leftShiftKey(key, size);
    }
}


void leftShiftKey(uint8_t* key, long size){
    uint8_t carry = 0;
    
    for(long i = size - 1; i >= 0; i--){
        uint8_t curByte = key[i];
        uint8_t tmpCarry = (curByte & 0x80) >> 7;
        key[i] = (curByte << 1) | carry;
        carry = tmpCarry;
    }
    
    // Roll over the leftmost bit to be the rightmost bit
    key[size-1] = key[size-1] | carry;
}


void encryptBlock(uint8_t* text, const uint8_t* key, long blockSize){
    for(long i = 0; i < blockSize; i++){
        text[i] = text[i] ^ key[i];
    }
}


void writeEncrypted(const uint8_t* encrypted, long length){
    fwrite(encrypted, sizeof(uint8_t), length, stdout);
}


long readInput(uint8_t* input, long length){
    long counter = 0;
    uint8_t byte;

    counter = fread(input, sizeof(uint8_t), length, stdin);

    return counter;
}


void* threadFunction(void* arg){
    // Threads variables data
    threadData* thData = (threadData*) arg;

    while(!isEmpty(thData->toEncrypt) || !isEmpty(thData->toWrite) || !thData->finishFlag){
        // Node to store the data from stdin/encrypted (encryptNode)
        Node* encryptNode = NULL;
        // Node to store the data to write to stdout (writeNode)
        Node* writeNode = NULL;

        // Try first to write to stdout if data is avliable
        writeNode = dequeue(thData->toWrite);
        if(writeNode != NULL){
            // If got a node but not in the right order to write, return it to the queue
            if(writeNode->blockNum != thData->blockNumber){
                int enqueuedNode = enqueueNode(thData->toWrite, writeNode);
                if(enqueuedNode == 0){
                    fprintf(stderr, "Error: Failed to enqueue a node.\n");
                    return NULL;
                }
            }
            // Else - write the data to stdout, increment the block number to be written, and free allocated memory
            else {
                pthread_mutex_lock(&mutexBlockNum);
                writeEncrypted(writeNode->data, writeNode->blockSize);
                thData->blockNumber++;
                pthread_mutex_unlock(&mutexBlockNum);

                free(writeNode->data);
                free(writeNode);
            }
        }

        // If there's plaintext data to be encrypted, do that
        if(!isEmpty(thData->toEncrypt)){
            encryptNode = dequeue(thData->toEncrypt);
        }
        // If not null, we have data to process
        if(encryptNode != NULL){
            // Get by how much we need to rotate the key
            long rotateAmount = encryptNode->blockNum % (encryptNode->blockSize * 8);
            // Allocate memory, and copy, to store the rotated key (original stays untouched)
            uint8_t* rotatedKey = (uint8_t*)malloc(thData->keySize);
            memcpy(rotatedKey, thData->key, thData->keySize);
            rotateKey(rotatedKey, rotateAmount, thData->keySize);
            
            // Encrypt the plaintext data
            encryptBlock(encryptNode->data, rotatedKey, encryptNode->blockSize);
            
            free(rotatedKey);

            // Put the node in the queue to be written
            if(enqueueNode(thData->toWrite, encryptNode) == 0){
                    fprintf(stderr, "Error: Failed to enqueue a node.\n");
                    return NULL;
                }
        }
    }

    return NULL;
}


char* processInput(int argc, char* argv[], int* threads){
    // checks number of arguments are valid
    if(argc != 5){
        fprintf(stderr, "Error: Wrong number of arguments. Please use ./program -n threadNum -key keyPath\n");
        return NULL;
    }

    char* path = NULL;
    // Assuming each processor has THREADS_PER_CORE to use. If user asks for more, raise an error.
    int maxThreads = get_nprocs() * THREADS_PER_CORE;

    for (int i = 1; i < argc; i++){
        // Search for the number of threads
        if (strcmp(argv[i], "-n") == 0 && i < argc - 1) {
            *threads = atoi(argv[++i]);
        }
        // Search for the the file path
        else if (strcmp(argv[i], "-k") == 0 && i < argc) {
            path = argv[++i];
        }
    }

    if(*threads < 0 || *threads > maxThreads || path == NULL){
        fprintf(stderr, "Error: In valid arguments were provided.\n");
        return NULL;
    }

    return path;
}


int main(int argc, char* argv[]){    
    int threadsNum = -1;
    char* keyfilePath = processInput(argc, argv, &threadsNum);

    if(keyfilePath == NULL){
        fprintf(stderr, "Error: In valid arguments were provided.\n");
        return 1;
    }

    // Try to read the the keyfile if succeed, we also get the block size (in bytes)
    long blockSize;
    uint8_t* key = readKeyFile(keyfilePath, &blockSize);
    if(!key){
        fprintf(stderr, "Error: Wasn't able to read the key file.\n");
        return 1;
    }
    if(blockSize == 0){
        fprintf(stderr,"Error: The key can't by an empty file (blocksize must be > 0)\n");
        return 1;
    }

    Queue* toEncrypt = createQueue();
    Queue* toWrite = createQueue();
    if(toEncrypt == NULL || toWrite == NULL){
        fprintf(stderr, "Error: Couldn't create a queue,\n");
        return 1;
    }

    pthread_mutex_init(&mutexBlockNum, NULL);

    // Structre to hold the queues data
    threadData thData; 
    thData.toEncrypt = toEncrypt;
    thData.toWrite = toWrite;
    thData.key = key;
    thData.keySize = blockSize;
    thData.blockNumber = 0;
    thData.finishFlag = 0;
    
    // Create N threads and send them to work
    pthread_t threads[threadsNum];
    for(int i = 0; i < threadsNum; i++){
        if (pthread_create(&threads[i], NULL, &threadFunction, (void*)&thData) != 0) {
            fprintf(stderr, "Error: Failed to create the thread(s)\n");
            return 1;
        }
    }

    // Array to store the input data (plaintex)
    uint8_t* inputData = (uint8_t*)malloc(blockSize);

    long blockNum = 0;
    long read;

    while(1){
        read = readInput(inputData, blockSize);
        // While we still read stdin data, get it, and put in the queue for encryption
        if(read > 0){
            int enqueued = enqueue(toEncrypt, inputData, read, blockNum);
            if(enqueued == 0){
                fprintf(stderr,"Error: Failed to enqueue the data.\n");
                return 1;
            }
            
            inputData = (uint8_t*)malloc(blockSize);
            if(inputData == NULL){
                fprintf(stderr, "Error: Failed to allocate inputData.\n");
                return 1;
            }
            blockNum++;

            // If the toEncrypt Q is getting to large (75% +), main thread rests (as longs as it's not the only thread)
            if(getSize(toEncrypt) >= (0.75 * MAX_QUEUE_SIZE) && threadsNum > 0){
                sleep(0.05);
            }
        }

        // Finished reading from stdin
        if(read < blockSize){
            thData.finishFlag = 1;
            break;
        }
    }

    // Main finished to read from stdin; goes to "help" encrypting and writing to stdout
    threadFunction((void*)&thData);

    // Wait for the threads to finish
    for (int i = 0; i < threadsNum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join the thread");
            return 1;
        }
    }

    // Clean up
    queueDistroyMutex(toEncrypt);
    queueDistroyMutex(toWrite);
    pthread_mutex_destroy(&mutexBlockNum);
    
    free(inputData);
    free(key);
    free(toEncrypt);
    free(toWrite);

    return 0;
}