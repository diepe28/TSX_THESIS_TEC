//
// Created by diego on 26/07/17.
//

#include "VectorMatrixMult.h"
#include "Queues.h"

#define DATATYPE long

__thread int _thread_id;

extern SectionQueue sectionQueue;
extern SimpleQueue simpleQueue;
extern SimpleSyncQueue simpleSyncQueue;
extern lynxQ_t lynxQ1;

extern CheckFrequency checkFrequency;
extern QueueType queueType;

extern volatile long producerCount;
extern volatile long consumerCount;

void printMatrix(int rows, int cols, long matrix[rows][cols]){
    int i,j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            printf("%ld ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void Vector_Matrix_Init(){
    int i, j;
    matrix = (int**) (malloc(sizeof(int*) * MATRIX_ROWS));

    for(i = 0; i < MATRIX_ROWS; i++){
        matrix[i] = (int*) (malloc(sizeof(int) * MATRIX_COLS));
    }

    for(i = 0; i < MATRIX_COLS; i++){
        vector[i] = rand() % 10000 +1;
//        if(i % 2)
//            vector[i] *= -1;
            //vector[i] = 0;
    }

    for(i = 0; i < MATRIX_ROWS; i++){
        for(j = 0; j < MATRIX_COLS; j++){
            matrix[i][j] = rand() % 10000 +1;
//            if(i % 2)
//                matrix[i][j] *= -1;
        }
    }
}

void Vector_Matrix_NotReplicated(){
    int i, j;

    for(i = 0; i < MATRIX_ROWS; i++){
        producerVectorResult[i] = 0;
        for(j = 0; j < MATRIX_COLS; j++){
            producerVectorResult[i] += vector[j] * matrix[i][j];
        }
    }
}

void Vector_Matrix_ReplicatedSameThread(){
    int i, j;
    long v1, v2;

    for(i = 0; i < MATRIX_ROWS; i++){
        producerVectorResult[i] = 0;
        for(j = 0; j < MATRIX_COLS; j++){
            v1 = vector[j] * matrix[i][j];
            v2 = vector[j] * matrix[i][j];
            if(v1 == v2){
                producerVectorResult[i] += v1;
            }else{
                printf("Soft error encountered!!!!\n\n");
                exit(1);
            }
        }
    }
}

// Simple Queue

void Vector_Matrix_SimpleQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, thisXOR = 0;
    long thisValues[MODULO];
    int j,k, modulo = 0, groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    producerVectorResult[i] += thisValue;
                    SimpleQueue_Enqueue(&simpleQueue, thisValue);
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            SimpleQueue_Enqueue(&simpleQueue, thisValues[k]);
                            producerVectorResult[i] += thisValues[k];
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        SimpleQueue_Enqueue(&simpleQueue, thisXOR);
                        modulo = 0;
                    }
                    producerVectorResult[i] += thisValue;
                }
            }
            break;
    }
}

void Vector_Matrix_SimpleQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, modulo = 0, j, k;
    long thisValues[MODULO], otherXOR = 0, thisXOR = 0;
    int groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    otherValue = SimpleQueue_Dequeue(&simpleQueue);

                    if(thisValue != otherValue){
                        printf("\n\n SOFT ERROR DETECTED \n\n");
                        exit(0);
                    }

                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            otherValue = SimpleQueue_Dequeue(&simpleQueue);
                            if(thisValues[k] != otherValue){
                                printf("\n\n SOFT ERROR DETECTED \n\n");
                                exit(0);
                            }
                            consumerVectorResult[i] += otherValue;
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        otherXOR = SimpleQueue_Dequeue(&simpleQueue);
                        if(thisXOR != otherXOR){
                            printf("\n\n SOFT ERROR DETECTED \n\n");
                            exit(0);
                        }
                        modulo = 0;
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;
    }

}

// Simple (without) Sync Queue

void Vector_Matrix_SimpleSyncQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, thisXOR = 0;
    long thisValues[MODULO];
    int j,k, modulo = 0, groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    producerVectorResult[i] += thisValue;
                    SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisValue);
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisValues[k]);
                            producerVectorResult[i] += thisValues[k];
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisXOR);
                        modulo = 0;
                    }
                    producerVectorResult[i] += thisValue;
                }
            }
            break;
    }
}

void Vector_Matrix_SimpleSyncQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, modulo = 0, j, k;
    long thisValues[MODULO], otherXOR = 0, thisXOR = 0;
    int groupLimit = MODULO -1, lastDeq;

    SetThreadAffinity(_thread_id);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                    if(thisValue != otherValue){
                        // Either is a soft error or a des-sync of the queue
                        while(simpleSyncQueue.content[simpleSyncQueue.deqPtr] == -2);
                        otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                        if(thisValue != otherValue) {
                            printf("\n\n SOFT ERROR DETECTED ");
                            printf("At time %ld... DeqPtr: %d EnqPtr: %d ConsumerCount: %ld ProducerCount: %ld This Value: %ld vs Value Read: %ld\n",
                                   clock(), simpleSyncQueue.deqPtr, simpleSyncQueue.enqPtr, consumerCount, producerCount, thisValue, otherValue);
                            exit(1);
                        }
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);
                            if(thisValues[k] != otherValue){
                                printf("\n\n SOFT ERROR DETECTED \n\n");
                                exit(0);
                            }
                            consumerVectorResult[i] += otherValue;
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                        if(thisXOR != otherXOR){
                            simpleSyncQueue.deqPtr = simpleSyncQueue.deqPtr == 0? SIMPLE_SYNC_QUEUE_SIZE -1 : simpleSyncQueue.deqPtr -1;

                            // barrier
                            //while(simpleSyncQueue.enqPtr == simpleSyncQueue.deqPtr);
                            //while(consumerCount >= producerCount);

                            otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                            if(thisXOR != otherXOR){
                                printf("\n\n SOFT ERROR DETECTED \n\n");
                                printf("EnqPtr: %d DeqPtr: %d ProducerCount: %ld ConsumerCount: %ld ThisValue: %ld vs OtherValue: %ld\n",
                                       simpleSyncQueue.enqPtr, simpleSyncQueue.deqPtr, producerCount, consumerCount,
                                       thisValue, otherValue
                                );
                                exit(1);
                            }
                        }
                        modulo = 0;
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;
    }
}

// Section Queue

void Vector_Matrix_SectionQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, thisXOR = 0;
    long thisValues[MODULO];
    int j,k, modulo = 0, groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    producerVectorResult[i] += thisValue;
                    SectionQueue_Enqueue(&sectionQueue, thisValue);
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            SectionQueue_Enqueue(&sectionQueue, thisValues[k]);
                            producerVectorResult[i] += thisValues[k];
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        SectionQueue_Enqueue(&sectionQueue, thisXOR);
                        modulo = 0;
                    }
                    producerVectorResult[i] += thisValue;
                }
            }
            break;
    }
}

void Vector_Matrix_SectionQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, modulo = 0, j, k;
    long thisValues[MODULO], otherXOR = 0, thisXOR = 0;
    int groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    otherValue = SectionQueue_Dequeue(&sectionQueue);

                    if(thisValue != otherValue){
                        printf("\n\n SOFT ERROR DETECTED \n\n");
                        exit(0);
                    }

                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            otherValue = SectionQueue_Dequeue(&sectionQueue);
                            if(thisValues[k] != otherValue){
                                printf("\n\n SOFT ERROR DETECTED \n\n");
                                exit(0);
                            }
                            consumerVectorResult[i] += otherValue;
                        }
                        modulo = 0;
                    }
                }
            }
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        otherXOR = SectionQueue_Dequeue(&sectionQueue);
                        if(thisXOR != otherXOR){
                            printf("\n\n SOFT ERROR DETECTED \n\n");
                            exit(0);
                        }
                        modulo = 0;
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;
    }

}

// Lynx Queue

void Vector_Matrix_LynxQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, thisXOR = 0;
    long thisValues[MODULO];
    int j,k, modulo = 0, groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    producerVectorResult[i] += thisValue;
                    queue_push_long(lynxQ1, thisValue);
                }
            }
            queue_push_done(lynxQ1);
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            queue_push_long(lynxQ1, thisValues[k]);
                            producerVectorResult[i] += thisValues[k];
                        }
                        modulo = 0;
                    }
                }
            }
            queue_push_done(lynxQ1);
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        queue_push_long(lynxQ1, thisXOR);
                        modulo = 0;
                    }
                    producerVectorResult[i] += thisValue;
                }
            }
            queue_push_done(lynxQ1);
            break;
    }
}

void Vector_Matrix_LynxQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, modulo = 0, j, k;
    long thisValues[MODULO], otherXOR = 0, thisXOR = 0;
    int groupLimit = MODULO -1;

    SetThreadAffinity(_thread_id);

    /* Busy wait until pop thread is ready to start */
    queue_busy_wait_pop_ready(lynxQ1);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    //otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error

                    if(thisValue != otherValue){
                        printf("\n\n SOFT ERROR DETECTED \n\n");
                        exit(0);
                    }

                    consumerVectorResult[i] += thisValue;
                }
            }
            queue_pop_done(lynxQ1);
            break;

        case CheckFrequency_eachModuloTimes:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValues[modulo] = vector[j] * matrix[i][j];

                    if(modulo++ == groupLimit){
                        for(k = 0; k < MODULO; k++) {
                            //otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error
                            if(thisValues[k] != otherValue){
                                printf("\n\n SOFT ERROR DETECTED \n\n");
                                exit(0);
                            }
                            consumerVectorResult[i] += otherValue;
                        }
                        modulo = 0;
                    }
                }
            }
            queue_pop_done(lynxQ1);
            break;

        case CheckFrequency_Encoding:
            for(i = 0; i < MATRIX_ROWS; i++){
                consumerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(modulo++ == groupLimit) {
                        //otherXOR = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error
                        if(thisXOR != otherXOR){
                            printf("\n\n SOFT ERROR DETECTED \n\n");
                            exit(0);
                        }
                        modulo = 0;
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            queue_pop_done(lynxQ1);
            break;
    }
}


void Vector_Matrix_ReplicatedThreads(int useHyperThread) {
    int err, numThreads;
    void *_start_routine;
    long i = 0;

    consumerCount = producerCount = 0;

    switch (queueType) {
        case QueueType_Simple:
            simpleQueue = SimpleQueue_Init();
            _start_routine = &Vector_Matrix_SimpleQueue_Consumer;
            break;

        case QueueType_SimpleSync:
            simpleSyncQueue = SimpleSyncQueue_Init();
            _start_routine = Vector_Matrix_SimpleSyncQueue_Consumer;
            break;

        case QueueType_Section:
            sectionQueue = SectionQueue_Init();
            _start_routine = &Vector_Matrix_SectionQueue_Consumer;
            break;

        case QueueType_lynxq:
            lynxQ1 = queue_init(LYNXQ_QUEUE_SIZE);
            _start_routine = &Vector_Matrix_LynxQueue_Consumer;
            break;
    }

    numThreads = 2;

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
                         (void *) (int64_t) (useHyperThread ? 2 : 1));

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }

    switch (queueType) {
        case QueueType_Simple:
            Vector_Matrix_SimpleQueue_Producer(0);
            break;
        case QueueType_SimpleSync:
            Vector_Matrix_SimpleSyncQueue_Producer(0);
            break;
        case QueueType_Section:
            Vector_Matrix_SectionQueue_Producer(0);
            break;
        case QueueType_lynxq:
            Vector_Matrix_LynxQueue_Producer(0);
            break;
    }

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);
}

void Vector_Matrix_MultAux(ExecMode executionMode) {
    int i, n;
    double times[NUM_RUNS], mean = 0, sd;
    long long matrixSum = 0;
    GTimer *timer;
    gulong fractional_part;
    gdouble milliseconds_elapsed;
    char queueTypeStr[30];
    char checkFrequencyStr[50];
    char executionModeStr[50];

    for(n = 0; n < NUM_RUNS; n++) {
        timer = g_timer_new();
        switch (executionMode) {
            case ExeMode_notReplicated:
                Vector_Matrix_NotReplicated();
                break;
            case ExeMode_replicatedSameThread:
                Vector_Matrix_ReplicatedSameThread();
                break;
            case ExeMode_replicatedThreads:
                Vector_Matrix_ReplicatedThreads(0);
                break;
            case ExeMode_replicatedHyperThreads:
                Vector_Matrix_ReplicatedThreads(1);
                break;
        }

        g_timer_stop(timer);
        fractional_part = 0;
        milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
        times[n] = milliseconds_elapsed;
        mean += milliseconds_elapsed;
        g_timer_destroy(timer);

        matrixSum = 0;
        if(executionMode == ExeMode_replicatedSameThread || executionMode == ExeMode_replicatedHyperThreads){
            for(i = 0; i < MATRIX_ROWS; i++){
                if(producerVectorResult[i] == consumerVectorResult[i])
                    matrixSum += producerVectorResult[i];
            }
        }else {
            for (i = 0; i < MATRIX_ROWS; i++) {
                matrixSum += producerVectorResult[i];
            }
        }

        SimpleSyncQueue_Destroy(&simpleSyncQueue);

        THREAD_UTILS_DestroyThreads();
        if (lynxQ1) {
            //lynx_queue_print_numbers();
            queue_finalize(lynxQ1);
            free(lynxQ1);
            lynxQ1 = NULL;
        }

        printf("Elapsed millisenconds: %f Matrix Sum: %lld\n", milliseconds_elapsed, matrixSum);
    }

    mean /= NUM_RUNS;

    for (i= 0; i < NUM_RUNS; i++) {
        sd += fabs(mean - times[i]);
    }

    sd /= NUM_RUNS;

    switch (queueType){
        case QueueType_Simple :
            strcpy(queueTypeStr, "SIMPLE");
            break;
        case QueueType_SimpleSync:
            strcpy(queueTypeStr, "SIMPLE SYNC");
            break;
        case QueueType_Section :
            strcpy(queueTypeStr, "SECTION");
            break;
        case QueueType_lynxq :
            strcpy(queueTypeStr, "LYNXQ");
            break;
    }

    switch (checkFrequency){
        case CheckFrequency_everyTime:
            sprintf(checkFrequencyStr, "checking everytime");
            break;
        case CheckFrequency_eachModuloTimes:
            sprintf(checkFrequencyStr, "checking every %d times", MODULO);
            break;
        case CheckFrequency_Encoding:
            sprintf(checkFrequencyStr, "checking every %d times with encoding", MODULO);
            break;
    }

    switch (executionMode) {
        case ExeMode_notReplicated:
            sprintf(executionModeStr, "Not replicated");
            break;
        case ExeMode_replicatedSameThread:
            sprintf(executionModeStr, "Replicated in Same Thread");
            break;
        case ExeMode_replicatedThreadsOptimally:
            sprintf(executionModeStr, "Replicated With Threads Optimally");
            break;
        case ExeMode_replicatedThreads:
            sprintf(executionModeStr, "Replicated With Threads");
            break;
        case ExeMode_replicatedHyperThreads:
            sprintf(executionModeStr, "Replicated With Hyper-Threads");
            break;
    }

    printf("--- Mean Execution Time of %s with %s queue %s: %f SD: %f\n\n",
           executionModeStr, queueTypeStr, checkFrequencyStr, mean, sd);
}

void Vector_Matrix_Mult(int useHyperThread){
    int i;
    Vector_Matrix_Init();

    Vector_Matrix_MultAux(ExeMode_notReplicated);
    //Vector_Matrix_MultAux(ExeMode_replicatedSameThread);

    // ----------------- SIMPLE QUEUE -----------------
    printf("--------------- Simple Queue Info, Size: %d \n\n", SIMPLE_QUEUE_MAX_ELEMENTS);
    Global_SetQueueType(QueueType_Simple);
    Global_SetCheckFrequency(CheckFrequency_everyTime);

    //Vector_Matrix_MultAux(ExeMode_replicatedThreads);
    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);
//
//    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);
//
//    Global_SetCheckFrequency(CheckFrequency_Encoding);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    // ----------------- SIMPLE SYNC QUEUE -----------------
    printf("--------------- Simple Sync Queue Info, Size: %d \n\n", SIMPLE_SYNC_QUEUE_SIZE);
    Global_SetQueueType(QueueType_SimpleSync);
//
    Global_SetCheckFrequency(CheckFrequency_everyTime);
    //Vector_Matrix_MultAux(ExeMode_replicatedThreads);
    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

//    Global_SetCheckFrequency(CheckFrequency_Encoding);
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    // ----------------- SECTION QUEUE -----------------
//    printf("--------------- Section Queue Info, Size Section: %d Number of Sections: %d \n\n", SECTION_SIZE, NUM_SECTIONS );
//    Global_SetQueueType(QueueType_Section);
//    Global_SetCheckFrequency(CheckFrequency_everyTime);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);
//
//    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

//    Global_SetCheckFrequency(CheckFrequency_Encoding);

//    if(!useHyperThread)
//        Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    else
//        Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    // ----------------- LYNXQ QUEUE -----------------
//    Global_SetQueueType(QueueType_lynxq);
//    Global_SetCheckFrequency(CheckFrequency_everyTime);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);
//
//    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);
//
//    Global_SetCheckFrequency(CheckFrequency_Encoding);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    for(i = 0; i < MATRIX_ROWS; i++){
        free(matrix[i]);
    }

    free(matrix);
}