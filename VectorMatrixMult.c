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
extern lynxQ_t lynxQ1;

extern CheckFrequency checkFrequency;
extern QueueType queueType;

extern volatile long producerCount;
extern volatile long consumerCount;

sem_t semProduce;
sem_t semCheck;

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
    }

}

// Simple (without) Sync Queue

void Vector_Matrix_SimpleSyncQueue_Producer(void* arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, j, thisValue;

    SetThreadAffinity(_thread_id);

    //display_thread_sched_attr("Attributes on producer thread");

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < MATRIX_ROWS; i++) {
                producerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];
                    producerVectorResult[i] += thisValue;
                    SimpleSyncQueue_Enqueue(&simpleSyncQueue, thisValue);
                }
            }
            break;

        case CheckFrequency_SynchronousSemaphores:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];

                    simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                    sem_post(&semProduce);

                    sem_wait(&semCheck);
                    producerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_SynchronousQueue:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];

                    simpleSyncQueue.content[simpleSyncQueue.enqPtr] = thisValue;
                    asm volatile("": : :"memory");
                    simpleSyncQueue.checkState = 0;

                    while(simpleSyncQueue.checkState == 0) asm("pause");

                    producerVectorResult[i] += thisValue;
                }
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
                        //usleep(10);
                        //pthread_yield();
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
    long i = 0, thisValue, otherValue, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];

                    otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);
                    if (thisValue != otherValue) {

                        // Either is a soft error or a des-sync of the queue
                        while (simpleSyncQueue.content[simpleSyncQueue.deqPtr] == ALREADY_CONSUMED); // asm("pause");

                        otherValue = SimpleSyncQueue_Dequeue(&simpleSyncQueue);

                        if (thisValue != otherValue) {
                            printf("\n\n SOFT ERROR DETECTED \n DeqPtr: %d EnqPtr: %d ConsumerCount: %ld "
                                           "ProducerCount: %ld This Value: %ld vs Value Read: %ld\n",
                                   simpleSyncQueue.deqPtr,
                                   simpleSyncQueue.enqPtr, consumerCount, producerCount, thisValue, otherValue);
                            exit(1);
                        }
                    }
                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_SynchronousSemaphores:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];

                    sem_wait(&semProduce);
                    if(simpleSyncQueue.content[simpleSyncQueue.enqPtr] != thisValue){
                        printf("\n\n SOFT ERROR DETECTED \n QueueValue[%d]: %ld vs thisValue: %ld\n",
                               simpleSyncQueue.enqPtr, simpleSyncQueue.content[simpleSyncQueue.enqPtr], thisValue);
                        exit(1);
                    }

                    sem_post(&semCheck);

                    consumerVectorResult[i] += thisValue;
                }
            }
            break;

        case CheckFrequency_SynchronousQueue:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];

                    while(simpleSyncQueue.checkState == 1) asm("pause");

                    if(simpleSyncQueue.content[simpleSyncQueue.enqPtr] != thisValue){
                        printf("\n\n SOFT ERROR DETECTED \n QueueValue[%d]: %ld vs thisValue: %ld\n",
                               simpleSyncQueue.enqPtr, simpleSyncQueue.content[simpleSyncQueue.enqPtr], thisValue);
                        exit(1);
                    }

                    simpleSyncQueue.enqPtr = (simpleSyncQueue.enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
                    asm volatile("": : :"memory");
                    simpleSyncQueue.checkState = 1;
                    consumerVectorResult[i] += thisValue;
                }
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
                        //usleep(10);
                        //pthread_yield();
                    }

//                    if(simpleSyncQueue.currentValue != thisValue){
//                        printf("\n\n SOFT ERROR DETECTED \n ");
//                        exit(1);
//                    }

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

//                    if(simpleSyncQueue.currentValue != thisValue){
//                        printf("\n\n SOFT ERROR DETECTED \n ");
//                        exit(1);
//                    }

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


long volatile iVol = 0, jVol = 0;
long volatile value1, value2;

void Vector_Matrix_Client(void* arg){
    _thread_id = (int) (int64_t) arg;

    SetThreadAffinity(_thread_id);

//    for (iVol = 0; iVol < MATRIX_ROWS; iVol++) {
//        consumerVectorResult[iVol] = 0;
//        for (jVol = 0; jVol < MATRIX_COLS; jVol++) {
//            __transaction_relaxed
//            {
//                value1 = vector[jVol] * matrix[iVol][jVol];
//                value2 = vector[jVol] * matrix[iVol][jVol];
//            }
//            if (value1 != value2) {
//                printf("\n\n SOFT ERROR DETECTED \n ");
//                exit(1);
//            }
//
//            consumerVectorResult[iVol] += value1;
//        }
//    }
}

// Lynx Queue

void Vector_Matrix_LynxQueue_Producer(void* arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, j;

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

        case CheckFrequency_SynchronousTwoVars:
            for(i = 0; i < MATRIX_ROWS; i++){
                producerVectorResult[i] = 0;
                for(j = 0; j < MATRIX_COLS; j++){
                    thisValue = vector[j] * matrix[i][j];
                    queue_push_long(lynxQ1, thisValue);
                    checkPerformed = 0;
                    while(checkPerformed == 0) asm("pause");
                    producerVectorResult[i] += thisValue;
                }
            }
            queue_push_done(lynxQ1);
            break;
    }
}

void Vector_Matrix_LynxQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, thisValue, otherValue, j;

    SetThreadAffinity(_thread_id);

    /* Busy wait until pop thread is ready to start */
    queue_busy_wait_pop_ready(lynxQ1);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];
                    otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error

                    if (thisValue != otherValue) {
                        printf("\n\n SOFT ERROR DETECTED \n\n");
                        exit(0);
                    }

                    consumerVectorResult[i] += thisValue;
                }
            }
            queue_pop_done(lynxQ1);
            break;

        case CheckFrequency_SynchronousTwoVars:
            for (i = 0; i < MATRIX_ROWS; i++) {
                consumerVectorResult[i] = 0;
                for (j = 0; j < MATRIX_COLS; j++) {
                    thisValue = vector[j] * matrix[i][j];
                    otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error

                    if (thisValue != otherValue) {
                        printf("\n\n SOFT ERROR DETECTED \n\n");
                        exit(0);
                    }
                    checkPerformed = 1;
                    consumerVectorResult[i] += thisValue;
                }
            }
            queue_pop_done(lynxQ1);
            break;
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

    sem_init(&semProduce, 0, 0);
    sem_init(&semCheck, 0, 0);

    switch (queueType) {
        case QueueType_Simple:
            simpleQueue = SimpleQueue_Init();
            _start_routine = &Vector_Matrix_SimpleQueue_Consumer;
            break;

        case QueueType_SimpleSync:
            simpleSyncQueue = SimpleSyncQueue_Init();
            _start_routine = Vector_Matrix_SimpleSyncQueue_Consumer;
            break;

        case QueueType_lynxq:
            lynxQ1 = queue_init(LYNXQ_QUEUE_SIZE);
            _start_routine = &Vector_Matrix_LynxQueue_Consumer;
            break;
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
        case QueueType_lynxq:
            Vector_Matrix_LynxQueue_Producer((void *) (int64_t)producerCore);
            break;
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
    char checkFrequencyStr[50];
    char executionModeStr[50];

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
        if (lynxQ1) {
            //lynx_queue_print_numbers();
            queue_finalize(lynxQ1);
            free(lynxQ1);
            lynxQ1 = NULL;
        }

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
        case CheckFrequency_SynchronousSemaphores:
            sprintf(checkFrequencyStr, "checking synchronously semaphores");
            break;
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
    Global_SetCheckFrequency(CheckFrequency_SynchronousTwoVars);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

//    Global_SetCheckFrequency(CheckFrequency_SynchronousQueue);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    Global_SetCheckFrequency(CheckFrequency_SynchronousOneVar);
    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

//    Global_SetCheckFrequency(CheckFrequency_SynchronousSemaphores);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    // ----------------- LYNXQ QUEUE -----------------
//    Global_SetQueueType(QueueType_lynxq);
//    Global_SetCheckFrequency(CheckFrequency_SynchronousTwoVars);
//
//    Vector_Matrix_MultAux(ExeMode_replicatedThreads);
//    Vector_Matrix_MultAux(ExeMode_replicatedHyperThreads);

    for(i = 0; i < MATRIX_ROWS; i++){
        free(matrix[i]);
    }

    free(matrix);
}