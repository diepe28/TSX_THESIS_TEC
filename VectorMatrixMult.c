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

volatile long currentValue;
volatile int checkPerformed = 0;
// Simple Queue

void Vector_Matrix_SimpleQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue;
    int j;

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

        default:
            printf("Something is wrong here!\n");
            exit(1);
    }
}

void Vector_Matrix_SimpleQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, j;

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

        default: break;
    }

}

// Simple (without) Sync Queue

void Vector_Matrix_SimpleSyncQueue_Producer(void* arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, j, thisValue, thisXOR, times = 0;

    SetThreadAffinity(_thread_id);

    //display_thread_sched_attr("Attributes on producer thread");

    switch (checkFrequency) {
        case CheckFrequency_SynchronousVolatile:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisValue);
                }

                simpleSyncQueue.currentValue = thisValue;
                simpleSyncQueue.checkState = 0;

                while (simpleSyncQueue.checkState == 0){
                    asm("pause");
                }

                // Pretend Volatile store
                producerVectorResult[i] = thisValue;
            }
            break;

        case CheckFrequency_SynchronousVolatileEncoding:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = thisXOR = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(times++ == MODULO){
                        SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisXOR);
                        times = 0;
                    }
                }

                // in case there are still values that have not been checked
                if(times != 0){
                    SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisXOR);
                }

                simpleSyncQueue.currentValue = thisValue;
                simpleSyncQueue.checkState = 0;

                while (simpleSyncQueue.checkState == 0){
                    asm("pause");
                }

                // Pretend Volatile store
                producerVectorResult[i] = thisValue;
            }
            break;

        case CheckFrequency_SynchronousTwoVars:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];

                    simpleSyncQueue.currentValue = thisValue;
                    simpleSyncQueue.checkState = 0;

                    while(simpleSyncQueue.checkState == 0) {
                        //producerCount++;
                        //asm("pause");
                    }

                    producerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_SynchronousOneVar:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];

                    simpleSyncQueue.currentValue = thisValue;

                    while(simpleSyncQueue.currentValue != ALREADY_CONSUMED) {
                        //producerCount++;
                        //asm("pause");
                    }
                    producerVectorResult[i] += thisValue;
                }
            }
            break;

        default:
            printf("Something is wrong here!\n");
            exit(1);
    }
}

void Vector_Matrix_SimpleSyncQueue_Consumer(void *arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, otherXOR, thisXOR, times = 0, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_SynchronousVolatile:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                    if (thisValue != otherValue) {
                        // des-sync of the queue
                        if(otherValue == ALREADY_CONSUMED) {
                            do{
                                asm("pause");
                            }
                            while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED);

                            otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                            if (thisValue == otherValue) continue;
                        }

                        printf("\n\n SOFT ERROR DETECTED \n");
                        exit(1);
                    }
                }

                while (simpleSyncQueue.checkState == 1){
                    asm("pause");
                }

                if(thisValue != simpleSyncQueue.currentValue){
                    printf("\n\n SOFT ERROR DETECTED \n");
                    exit(1);
                }

                // Pretend Volatile Store
                consumerVectorResult[i] = thisValue;
                simpleSyncQueue.checkState = 1;
            }
            break;

        case CheckFrequency_SynchronousVolatileEncoding:
            for (i = 0; i < MATRIX_ROWS; i++) {
                thisValue = thisXOR = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue += vector[j] * matrix[i][j];
                    thisXOR ^= thisValue;

                    if(times++ == MODULO) {
                        otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);
                        times = 0;
                        if (thisXOR != otherXOR) {
                            // des-sync of the queue
                            if (otherXOR == ALREADY_CONSUMED) {
                                do {
                                    asm("pause");
                                } while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED);

                                otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                                if (thisXOR == otherXOR) continue;
                            }

                            printf("\n\n SOFT ERROR DETECTED \n");
                            exit(1);
                        }
                    }
                }

                // in case there are still values that have not been checked
                if(times != 0){
                    otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);
                    if (thisXOR != otherXOR) {
                        // des-sync of the queue
                        if (otherXOR == ALREADY_CONSUMED) {
                            do {
                                asm("pause");
                            } while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED);

                            otherXOR = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                            if (thisXOR != otherXOR){
                                printf("\n\n SOFT ERROR DETECTED \n");
                                exit(1);
                            }
                        }else{
                            printf("\n\n SOFT ERROR DETECTED \n");
                            exit(1);
                        }
                    }
                }

                while (simpleSyncQueue.checkState == 1){
                    asm("pause");
                }

                if(thisValue != simpleSyncQueue.currentValue){
                    printf("\n\n SOFT ERROR DETECTED \n");
                    exit(1);
                }

                // Pretend Volatile Store
                consumerVectorResult[i] = thisValue;
                simpleSyncQueue.checkState = 1;
            }
            break;



        case CheckFrequency_SynchronousTwoVars:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];

                    while(simpleSyncQueue.checkState == 1) {
                        //consumerCount++;
                        //asm("pause");
                    }

                    simpleSyncQueue.checkState = 1;
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_SynchronousOneVar:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];

                    while(simpleSyncQueue.currentValue == ALREADY_CONSUMED) {
                        //consumerCount++;
                        //asm("pause");
                    }

                    simpleSyncQueue.currentValue = ALREADY_CONSUMED;
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        default:
            printf("Something is wrong here!\n");
            exit(1);

    }
}

// Test Structure

void CreateThreadsWithScheduling(void *_start_routine, int consumerCore){
    int err = 0;
    pthread_attr_t attr;
    struct sched_param param;

    // For producer/main thread
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    err = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    if (err != 0) handle_error_en(err, "producer_setschedparam");

    // For the consumer thread
    //param.sched_priority = sched_get_priority_max(SCHED_RR);

    err = pthread_attr_init(&attr);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_init");

    err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (err != 0) handle_error_en(err, "pthread_attr_setinheritsched");

    err = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_setschedpolicy");

    err = pthread_attr_setschedparam(&attr, &param);
    if (err != 0) handle_error_en(err, "consumer pthread_attr_setschedparam");

    err = pthread_create(THREAD_UTILS_Threads[1], &attr, _start_routine, (void *) (int64_t) consumerCore);

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }
}

void Vector_Matrix_ReplicatedThreads(int useHyperThread) {
    int err = 0, numThreads;
    void *_start_routine = 0;
    long i = 0;
    int producerCore = 0;
    int consumerCore = useHyperThread? producerCore +2 : producerCore +1;;

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
        default: break;
    }

    numThreads = 2;

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    //CreateThreadsWithScheduling(_start_routine, consumerCore);
    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine, (void *) (int64_t) consumerCore);

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }

    switch (queueType) {
        case QueueType_Simple:
            Vector_Matrix_SimpleQueue_Producer((void *) (int64_t) producerCore);
            break;
        case QueueType_SimpleSync:
            Vector_Matrix_SimpleSyncQueue_Producer((void *) (int64_t)producerCore);
            break;
        default: break;
    }

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);
}

double baseLineMean;

void Vector_Matrix_MultAux(ExecMode executionMode) {
    int i, n;
    double times[NUM_RUNS], meanTime = 0, sd, meanConWait = 0, meanProWait = 0;
    long long matrixSum = 0;
    GTimer *timer;
    gulong fractional_part;
    gdouble milliseconds_elapsed;
    char queueTypeStr[30];
    char checkFrequencyStr[60];
    char executionModeStr[60];

    for(n = 0; n < NUM_RUNS; n++) {
        consumerCount = producerCount = 0;
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
            default: break;
        }

        g_timer_stop(timer);
        fractional_part = 0;
        milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
        times[n] = milliseconds_elapsed;
        meanTime += milliseconds_elapsed;
        g_timer_destroy(timer);
        meanConWait += consumerCount;
        meanProWait += producerCount;

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

        if(queueType == QueueType_SimpleSync)
            SimpleSyncQueue_Destroy(&simpleSyncQueue);

        THREAD_UTILS_DestroyThreads();

        printf("Elapsed millisenconds: %f Matrix Sum: %lld ... Consumer waiting: %ld ... Producer waiting: %ld\n",
               milliseconds_elapsed, matrixSum, consumerCount, producerCount);
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

    switch (queueType){
        case QueueType_Simple :
            strcpy(queueTypeStr, "SIMPLE");
            break;
        case QueueType_SimpleSync:
            strcpy(queueTypeStr, "SIMPLE SYNC");
            break;
        case QueueType_lynxq :
            strcpy(queueTypeStr, "LYNXQ");
            break;
        default: break;
    }

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
        case CheckFrequency_SynchronousVolatile:
            sprintf(checkFrequencyStr, "checking synchronously volatile stores");
            break;
        case CheckFrequency_SynchronousVolatileEncoding:
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
            sprintf(executionModeStr, "Replicated With Threads");
            break;
        case ExeMode_replicatedHyperThreads:
            sprintf(executionModeStr, "Replicated With Hyper-Threads");
            break;
        default: break;
    }

    printf("--- Mean Execution Time of %s with %s queue %s: %f SD: %f Relative to baseline: %fx\n "
           "--- MeanConsumerWaiting: %f  MeanProducerWaiting: %f \n\n",
           executionModeStr, queueTypeStr, checkFrequencyStr, meanTime, sd, meanTime / baseLineMean, meanConWait, meanProWait);
}

void Vector_Matrix_Mult(int useHyperThread) {
    int i;
    Vector_Matrix_Init();

    Vector_Matrix_MultAux(ExeMode_notReplicated);
    //Vector_Matrix_MultAux(ExeMode_replicatedSameThread);

    // ----------------- SIMPLE SYNC QUEUE -----------------
//    printf("--------------- Simple Sync Queue Info, Size: %d \n\n", SIMPLE_SYNC_QUEUE_SIZE);
    Global_SetQueueType(QueueType_SimpleSync);
//    Global_SetCheckFrequency(CheckFrequency_SynchronousVolatile);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    Global_SetCheckFrequency(CheckFrequency_SynchronousVolatileEncoding);
    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    for(i = 0; i < MATRIX_ROWS; i++){
        free(matrix[i]);
    }

    free(matrix);
}