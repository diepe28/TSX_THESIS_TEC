//
// Created by diego on 26/07/17.
//

#include "VectorMatrixMult.h"
#include "Queues.h"


#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

__thread int _thread_id;

extern SectionQueue sectionQueue;
extern SimpleQueue simpleQueue;
extern SimpleSyncQueue simpleSyncQueue;

extern CheckFrequency checkFrequency;
extern QueueType queueType;

extern volatile long producerCount;
extern volatile long consumerCount;

static void display_sched_attr(int policy, struct sched_param *param) {
    printf("    policy=%s, priority=%d\n\n",
           (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
           (policy == SCHED_RR)    ? "SCHED_RR" :
           (policy == SCHED_OTHER) ? "SCHED_OTHER" :
           "???",
           param->sched_priority);
}

static void display_thread_sched_attr(char *msg) {
    int policy, s;
    struct sched_param param;

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    if (s != 0)
        handle_error_en(s, "pthread_getschedparam");

    printf("%s ", msg);
    display_sched_attr(policy, &param);
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

void Vector_Matrix_SimpleSyncQueue_Consumer(void *arg);

void Vector_Matrix_SimpleSyncQueue_Producer(void* arg) {
    _thread_id = (int) (int64_t) arg;
    long thisValue,thisXOR, times = 0;
    int localDeqPtr, newLimit, nextEnq, firstLimit, i, j;

    SetThreadAffinity(_thread_id);

    //display_thread_sched_attr("Attributes on producer thread");

    switch (checkFrequency) {
        case CheckFrequency_RHT_Volatiles:
            newLimit = SIMPLE_SYNC_QUEUE_SIZE;

            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = j = 0;
                // this is one BIG assumption: SIMPLE_SYNC_QUEUE_SIZE < MATRIX_COLS

                do {
                    for (; j < newLimit; j++) {
                        thisValue += vector[j] * matrix[i][j];
                        simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                        simpleSyncQueue.enqPtr = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                    }

                    nextEnq = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
//                    if(simpleSyncQueue.content[nextEnq] != ALREADY_CONSUMED) {
//                        producerCount++;
                        while (simpleSyncQueue.content[nextEnq] != ALREADY_CONSUMED) {
                            asm("pause");
                        }
//                    }

                    localDeqPtr = simpleSyncQueue.deqPtr;
                    newLimit += simpleSyncQueue.enqPtr >= localDeqPtr ?
                                (SIMPLE_SYNC_QUEUE_SIZE - simpleSyncQueue.enqPtr) + localDeqPtr :
                                localDeqPtr - simpleSyncQueue.enqPtr;

                } while (newLimit < MATRIX_COLS);

                // iterations left
                for (; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                    simpleSyncQueue.enqPtr = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                }

                simpleSyncQueue.volatileValue = thisValue;
                simpleSyncQueue.checkState = 0;
                while (simpleSyncQueue.checkState == 0) {
                    asm("pause");
                }
                // Pretend Volatile store
                producerVectorResult[i] = thisValue;
                newLimit = SIMPLE_SYNC_QUEUE_SIZE;
            }
            break;

        case CheckFrequency_RHT_VolatilesImproved:
            newLimit = SIMPLE_SYNC_QUEUE_SIZE-1;
            localDeqPtr = 0;

            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = j = 0;
                // this is one BIG assumption: SIMPLE_SYNC_QUEUE_SIZE < MATRIX_COLS

                do {
                    for (; j < newLimit; j++) {
//                        nextEnq = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
//                        simpleSyncQueue.content[nextEnq] = ALREADY_CONSUMED;

                        thisValue += vector[j] * matrix[i][j];
                        simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                        simpleSyncQueue.enqPtr = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;;
                    }

                    if(localDeqPtr == simpleSyncQueue.deqPtr) {
                        producerCount++;
                        while (localDeqPtr == simpleSyncQueue.deqPtr) {
                            asm("pause");
                        }
                    }

                    localDeqPtr = simpleSyncQueue.deqPtr;

                    newLimit += (simpleSyncQueue.enqPtr >= localDeqPtr ?
                                (SIMPLE_SYNC_QUEUE_SIZE - simpleSyncQueue.enqPtr) + localDeqPtr :
                                localDeqPtr - simpleSyncQueue.enqPtr) -1;

                } while (newLimit < MATRIX_COLS);

                // iterations left

                for (; j < MATRIX_COLS; j++) {
//                    nextEnq = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
//                    simpleSyncQueue.content[nextEnq] = ALREADY_CONSUMED;

                    thisValue += vector[j] * matrix[i][j];
                    simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                    simpleSyncQueue.enqPtr = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                }

                simpleSyncQueue.volatileValue = thisValue;
                simpleSyncQueue.checkState = 0;
                while (simpleSyncQueue.checkState == 0) {
                    asm("pause");
                }
                // Pretend Volatile store
                producerVectorResult[i] = thisValue;
                newLimit = SIMPLE_SYNC_QUEUE_SIZE - 1;
                localDeqPtr = simpleSyncQueue.enqPtr;
            }
            break;

        case CheckFrequency_Volatile:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisValue);
                }

                simpleSyncQueue.volatileValue = thisValue;
                simpleSyncQueue.checkState = 0;

                while (simpleSyncQueue.checkState == 0){
                    asm("pause");
                }

                // Pretend Volatile store
                producerVectorResult[i] = thisValue;
            }

            break;

        default:
            printf("Something is wrong here!\n");
            exit(1);
    }
}

static void inline ConsumerCheckAndMoveImproved(long currentValue){
    long otherValue = simpleSyncQueue.content[simpleSyncQueue.deqPtr];

    if (currentValue != otherValue) {
        // des-sync of the queue
        if (otherValue == ALREADY_CONSUMED) {
            consumerCount++;
            do {
                asm("pause");
            } while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED);

            otherValue = simpleSyncQueue.content[simpleSyncQueue.deqPtr];

            if (currentValue == otherValue){
//                simpleSyncQueue.content[simpleSyncQueue.deqPtr] = ALREADY_CONSUMED;
                simpleSyncQueue.deqPtr = (simpleSyncQueue.deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                return;
            }
        }

        printf("\n\n SOFT ERROR DETECTED, %ld vs %ld Producer: %ld -- Consumer: %ld, diff: %ld \n",
               currentValue, otherValue, producerCount, consumerCount, producerCount - consumerCount);
        exit(1);
    }else{
//        simpleSyncQueue.content[simpleSyncQueue.deqPtr] = ALREADY_CONSUMED;
        simpleSyncQueue.deqPtr = (simpleSyncQueue.deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
    }
}

static void inline ConsumerCheckAndMove(long currentValue){
    long otherValue = simpleSyncQueue.content[simpleSyncQueue.deqPtr];

    if (currentValue != otherValue) {
        // des-sync of the queue
        if (otherValue == ALREADY_CONSUMED) {
            consumerCount++;
            do {
                asm("pause");
            } while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED);

            otherValue = simpleSyncQueue.content[simpleSyncQueue.deqPtr];

            if (currentValue == otherValue){
                simpleSyncQueue.content[simpleSyncQueue.deqPtr] = ALREADY_CONSUMED;
                simpleSyncQueue.deqPtr = (simpleSyncQueue.deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                return;
            }
        }

        printf("\n\n SOFT ERROR DETECTED, %ld vs %ld Producer: %ld -- Consumer: %ld, diff: %ld \n",
               currentValue, otherValue, producerCount, consumerCount, producerCount - consumerCount);
        exit(1);
    }else{
        simpleSyncQueue.content[simpleSyncQueue.deqPtr] = ALREADY_CONSUMED;
        simpleSyncQueue.deqPtr = (simpleSyncQueue.deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
    }
}

void Vector_Matrix_SimpleSyncQueue_Consumer(void *arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherXOR, thisXOR, times = 0, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_Volatile:
        case CheckFrequency_RHT_Volatiles:
        case CheckFrequency_VolatileNoSyncNoModulo:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    ConsumerCheckAndMove(thisValue);
                }

                while (simpleSyncQueue.checkState == 1) {
                    asm("pause");
                }

                if (thisValue != simpleSyncQueue.volatileValue) {
                    printf("\n\n SOFT ERROR IN VOLATILE STORE \n");
                    exit(1);
                }

                simpleSyncQueue.checkState = 1;
                // Pretend Volatile Store
                consumerVectorResult[i] = thisValue;
            }
            break;

        case CheckFrequency_RHT_VolatilesImproved:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    ConsumerCheckAndMoveImproved(thisValue);
                }

                while (simpleSyncQueue.checkState == 1) {
                    asm("pause");
                }

                if (thisValue != simpleSyncQueue.volatileValue) {
                    printf("\n\n SOFT ERROR IN VOLATILE STORE \n");
                    exit(1);
                }

                simpleSyncQueue.checkState = 1;
                // Pretend Volatile Store
                consumerVectorResult[i] = thisValue;
            }
            break;

        default:
            printf("Something is wrong here!\n");
            exit(1);
    }
}

// Test Structure

void CreateThreadsWithScheduling(void *_start_routine, int consumerCore){
    int err = 0, scheduler = SCHED_RR;
    pthread_attr_t attr;
    struct sched_param param;


    // For producer/main thread
    param.sched_priority = sched_get_priority_min(scheduler) + 30;

    err = pthread_setschedparam(pthread_self(), scheduler, &param);
    if (err != 0) handle_error_en(err, "producer_setschedparam");

    // For the consumer thread
    param.sched_priority = sched_get_priority_max(scheduler) - 10;

    err = pthread_attr_init(&attr);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_init");

    err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (err != 0) handle_error_en(err, "pthread_attr_setinheritsched");

    err = pthread_attr_setschedpolicy(&attr, scheduler);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_setschedpolicy");

    err = pthread_attr_setschedparam(&attr, &param);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_setschedparam");

    err = pthread_create(THREAD_UTILS_Threads[1], &attr, _start_routine, (void *) (int64_t) consumerCore);

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }
}

void Vector_Matrix_ReplicatedThreads(int producerCore, int consumerCore) {
    int err = 0, numThreads;
    void *_start_routine = 0;

    consumerCount = producerCount = 0;

    simpleSyncQueue = SimpleSyncQueue_Init();
    _start_routine = Vector_Matrix_SimpleSyncQueue_Consumer;

    numThreads = 2;

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

//    CreateThreadsWithScheduling(_start_routine, consumerCore);
    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine, (void *) (int64_t) consumerCore);

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }

    Vector_Matrix_SimpleSyncQueue_Producer((void *) (int64_t)producerCore);

    pthread_join(*THREAD_UTILS_Threads[1], NULL);
}

double baseLineMean;

void Vector_Matrix_MultAux(ExecMode executionMode, int producerCore, int consumerCore) {
    int i, n;
    double times[NUM_RUNS], meanTime = 0, sd, meanConWait = 0, meanProWait = 0, milliseconds_elapsed;
    struct timespec start, finish;
    long long matrixSum = 0;
    char checkFrequencyStr[60];
    char executionModeStr[60];


    for(n = 0; n < NUM_RUNS; n++) {
        consumerCount = producerCount = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);

        switch (executionMode) {
            case ExeMode_notReplicated:
                Vector_Matrix_NotReplicated();
                break;
            case ExeMode_replicatedSameThread:
                Vector_Matrix_ReplicatedSameThread();
                break;
            case ExeMode_replicatedThreads:
                Vector_Matrix_ReplicatedThreads(producerCore, consumerCore);
                break;
            default: break;
        }

        clock_gettime(CLOCK_MONOTONIC, &finish);
        milliseconds_elapsed = (finish.tv_sec - start.tv_sec + (finish.tv_nsec - start.tv_nsec) / 1000000000.0) * 1000;
        times[n] = milliseconds_elapsed;
        meanTime += milliseconds_elapsed;
        meanConWait += consumerCount;
        meanProWait += producerCount;

        matrixSum = 0;
        if(executionMode == ExeMode_replicatedSameThread){
            for(i = 0; i < MATRIX_ROWS; i++){
                if(producerVectorResult[i] == consumerVectorResult[i])
                    matrixSum += producerVectorResult[i];
            }
        }else {
            for (i = 0; i < MATRIX_ROWS; i++) {
                matrixSum += producerVectorResult[i];
            }
        }

        if(queueType == QueueType_SimpleSync)
            SimpleSyncQueue_Destroy(&simpleSyncQueue);

        THREAD_UTILS_DestroyThreads();

        printf("Millisenconds: %f, MatrixSum: %lld ... Consumer: %ld , Ratio %f --- Producer: %ld Ratio: %f\n",
               milliseconds_elapsed,  matrixSum, consumerCount, ((double) consumerCount) / (MATRIX_COLS * MATRIX_ROWS),
                producerCount, ((double) producerCount) / (MATRIX_COLS * MATRIX_ROWS));
    }

    meanTime /= NUM_RUNS;
    meanConWait /= NUM_RUNS;
    meanProWait /= NUM_RUNS;

    for (i= 0; i < NUM_RUNS; i++) {
        sd += fabs(meanTime - times[i]);
    }

    sd /= NUM_RUNS;

    if(executionMode == ExeMode_notReplicated)
        baseLineMean = meanTime;

    switch (checkFrequency){
        case CheckFrequency_everyTime:
            sprintf(checkFrequencyStr, "checking everytime");
            break;
        case CheckFrequency_SynchronousTwoVars:
            sprintf(checkFrequencyStr, "checking synchronously");
            break;
        case CheckFrequency_SynchronousOneVar:
            sprintf(checkFrequencyStr, "checking synchronously with just one sync var");
            break;
        case CheckFrequency_SynchronousQueue:
            sprintf(checkFrequencyStr, "checking synchronously with the queue");
            break;
        case CheckFrequency_Volatile:
            sprintf(checkFrequencyStr, "checking synchronously volatile stores");
            break;
        case CheckFrequency_RHT_Volatiles:
            sprintf(checkFrequencyStr, "RHT with volatile stores");
            break;
        case CheckFrequency_RHT_NoVolatiles:
            sprintf(checkFrequencyStr, "RHT with NO volatile stores");
            break;
        case CheckFrequency_RHT_VolatilesImproved:
            sprintf(checkFrequencyStr, "RHT with volatile stores IMPROVED");
            break;
        case CheckFrequency_VolatileNoSyncNoModulo:
            sprintf(checkFrequencyStr, "synchronously on volatile stores no sync no modulo");
            break;
        case CheckFrequency_VolatileEncoding:
            sprintf(checkFrequencyStr, "checking synchronously volatile stores encoding");
            break;
        default: break;
    }

    switch (executionMode) {
        case ExeMode_notReplicated:
            sprintf(executionModeStr, "Not replicated");
            break;
        case ExeMode_replicatedSameThread:
            sprintf(executionModeStr, "Replicated in Same Thread");
            break;
        case ExeMode_replicatedThreads:
            sprintf(executionModeStr, "Replicated in Threads %d, %d", producerCore, consumerCore);
            break;
        default: break;
    }

    printf("--- Mean Execution Time of %s with %s: %f SD: %f Relative to baseline: %fx\n "
           "--- MeanConsumerWaiting: %f  MeanProducerWaiting: %f \n\n",
           executionModeStr, checkFrequencyStr, meanTime, sd, meanTime / baseLineMean,
           meanConWait, meanProWait);
}

void Vector_Matrix_Mult(int producerCore, int consumerCore, int producerCoreHT, int consumerCoreHT){
    int i;
    Vector_Matrix_Init();
    Global_SetQueueType(QueueType_SimpleSync);


    Vector_Matrix_MultAux(ExeMode_notReplicated, 0 , 0);

    // ----------------- SIMPLE SYNC QUEUE -----------------
//    printf("--------------- Simple Sync Queue Info, Size: %d \n\n", SIMPLE_SYNC_QUEUE_SIZE);

//    Global_SetCheckFrequency(CheckFrequency_RHT_Volatiles);
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCore, consumerCore);
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCoreHT, consumerCoreHT);

    Global_SetCheckFrequency(CheckFrequency_RHT_VolatilesImproved);
    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCore, consumerCore);
    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCoreHT, consumerCoreHT);

//    Global_SetCheckFrequency(CheckFrequency_RHT_Volatiles);
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCore, consumerCore);
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads, producerCoreHT, consumerCoreHT);

    for(i = 0; i < MATRIX_ROWS; i++){
        free(matrix[i]);
    }

    free(matrix);
}