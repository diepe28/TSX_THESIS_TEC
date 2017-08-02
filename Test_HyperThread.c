//
// Created by diego on 26/07/17.
//

#include "Test_HyperThread.h"

__thread int _thread_id;
int sharedValue = 0;

void SetThreadAffinity(int threadId){
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(threadId, &cpuset);

    if(THREAD_UTILS_NUM_THREADS > 1) {
        /* pin the thread to a core */
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
            fprintf(stderr, "Thread pinning failed!\n");
            exit(1);
        }
    }
}

void* PingPong(void * arg){
    _thread_id = (int) (int64_t) arg;
    int i = 0;
    SetThreadAffinity(_thread_id);

    for(; i < 100000000; i++){

        while(sharedValue == _thread_id);
        sharedValue = _thread_id;
    }

}

void HyperThreads_PingPongTest(int useHyperThread) {
    int i, err, numThreads;
    void *_start_routine = &PingPong;

    numThreads = 2;
    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    GTimer *timer = g_timer_new();

    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
                         (void*) (int64_t) ((useHyperThread) ? 2 : 1));

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", i);
        exit(1);
    }

    PingPong(0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    THREAD_UTILS_DestroyThreads();

    printf("Total number of seconds %f\n", seconds_elapsed);
}

//////////////////////// QUEUE TESTS ////////////////////////////
SectionQueue sectionQueue;
bool checkingEachModuleTime = false;

long CalcFunction(int index, long value){
    return index % 2 ? value * 2 : -value * 2;
}

void TestQueue_ThreadProducerOptimal(void * arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, result = 0;

    SetThreadAffinity(_thread_id);

    for (i = 0; i < NUM_VALUES; i++) {
        auxValue = CalcFunction(i, values[i]);
        result += auxValue;
        //printf("Enqueue %ld %ld\n", i, auxValue);
    }

}

void TestQueue_ThreadConsumerOptimal(void * arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;


    SetThreadAffinity(_thread_id);

    for (i = 0; i < NUM_VALUES; i++) {
        auxValue = CalcFunction(i, values[i]);
        otherValue = auxValue;

        if (auxValue != otherValue) {
            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i,
                   values[i], auxValue, otherValue);
            exit(1);
        }

        result += otherValue;
        //printf("Dequeue Value %ld %ld\n", i, otherValue);
    }

}

#define MODULO 10

void TestQueue_ThreadProducer(void * arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, result = 0;

    SetThreadAffinity(_thread_id);

    ///////////// Enqueing MODULO times each time
    if(checkingEachModuleTime){
        long auxValues[MODULO];
        int modulo, j;

        for (i = 0; i < NUM_VALUES; i++){
            modulo = i % MODULO;

            auxValues[modulo] = CalcFunction(i, values[i]);
            result += auxValues[modulo];

            if (modulo == MODULO-1)
                for(j = 0; j < MODULO; j++)
                    SectionQueue_Enqueue(&sectionQueue, auxValues[j]);
        }
    } else{ ///////////// Enqueing every time
        for (i = 0; i < NUM_VALUES; i++) {
            auxValue = CalcFunction(i, values[i]);
            SectionQueue_Enqueue(&sectionQueue, auxValue);
            result += auxValue;
            //printf("Enqueue %ld %ld\n", i, auxValue);
        }
    }
}

void TestQueue_ThreadConsumer(void * arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    ///////////// Dequeuing each MODULO times
    if(checkingEachModuleTime) {
        for (i = 0; i < NUM_VALUES; i++) {
            modulo = i % MODULO;
            auxValues[modulo] = CalcFunction(i, values[i]);

            if (modulo == MODULO - 1) {
                for (j = 0; j < MODULO; j++) {
                    otherValue = SectionQueue_Dequeue(&sectionQueue);

                    if (auxValues[j] != otherValue) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n",
                               i,
                               values[i], auxValue, otherValue);
                        exit(1);
                    }
                    result += otherValue;
                }
            }
        }
    } else { ///////////// Dequeuing Every time
        for (i = 0; i < NUM_VALUES; i++) {
        auxValue = CalcFunction(i, values[i]);
        otherValue = SectionQueue_Dequeue(&sectionQueue);

        if(auxValue != otherValue){
            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i, values[i], auxValue, otherValue);
            exit(1);
        }

        result += otherValue;
        //printf("Dequeue Value %ld %ld\n", i, otherValue);
        }
    }

}

double HyperThreads_QueueTestReplicatedOptimally() {
    int err, numThreads;
    void *_start_routine = &TestQueue_ThreadConsumerOptimal;
    long i;

    bool useHyperThread = false;
    numThreads = 2;

    // random values
    for (i = 0; i < NUM_VALUES; i++) {
        values[i] = rand() % 1000000;
    }

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    GTimer *timer = g_timer_new();

    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
                         (void *) (int64_t) ((useHyperThread) ? 2 : 1));

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }


    TestQueue_ThreadProducerOptimal(0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    THREAD_UTILS_DestroyThreads();

    //SimpleQueue_WastedInst();
    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

double HyperThreads_QueueTestReplicated(int useHyperThread) {
    int err, numThreads;
    void *_start_routine = &TestQueue_ThreadConsumer;
    long i;

    sectionQueue = SectionQueue_Init();
    numThreads = 2;

    // random values
    for (i = 0; i < NUM_VALUES; i++) {
        values[i] = rand() % 1000000;
    }

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    GTimer *timer = g_timer_new();

    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
                         (void *) (int64_t) ((useHyperThread) ? 2 : 1));

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", 1);
        exit(1);
    }


    TestQueue_ThreadProducer(0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    THREAD_UTILS_DestroyThreads();

    //SimpleQueue_WastedInst();
    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

double HyperThreads_QueueTestNotReplicated() {
    long i, auxValue, result = 0;

    // random values
    for (i = 0; i < NUM_VALUES; i++) {
        values[i] = rand() % 1000000;
    }

    GTimer *timer = g_timer_new();

    for (i = 0; i < NUM_VALUES; i++){
        auxValue = CalcFunction(i, values[i]);
        result += auxValue;
    }

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

double HyperThreads_SameThreadReplicated(){
    long i, v1, v2,  result = 0;

    // random values
    for (i = 0; i < NUM_VALUES; i++) {
        values[i] = rand() % 1000000;
    }

    GTimer *timer = g_timer_new();

    for (i = 0; i < NUM_VALUES; i++){
        v1 = CalcFunction(i, values[i]);
        v2 = CalcFunction(i, values[i]);

        if(v1 != v2){
            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i, values[i], v1, v2);
            exit(1);
        }

        result += v1;
    }

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

void HyperThreads_QueueTest(ExecMode execMode){
    int i = 0, n = 5;
    double result = 0;

    checkingEachModuleTime = false;

    if(execMode == replicated_CheckImproved){
        execMode = replicatedThreads;
        checkingEachModuleTime = true;
    }else{
        if (execMode == replicatedHT_CheckImproved){
            execMode = replicatedHT;
            checkingEachModuleTime = true;
        }
    }

    for (; i < n; i++) {
        switch (execMode) {
            case notReplicated:
                result += HyperThreads_QueueTestNotReplicated();
                break;
            case replicatedSameThread:
                result += HyperThreads_SameThreadReplicated();
                break;
            case replicatedThreadsOptimally:
                result += HyperThreads_QueueTestReplicatedOptimally();
                break;
            case replicatedThreads:
                result += HyperThreads_QueueTestReplicated(0);
                break;
            case replicatedHT:
                result += HyperThreads_QueueTestReplicated(1);
                break;
        }
    }

    switch (execMode) {
        case notReplicated:
            printf("\n---------- Not replicated: ");
            break;
        case replicatedSameThread:
            printf("\n---------- Replicated in the same thread: ");
            break;
        case replicatedThreadsOptimally:
            printf("\n---------- Replicated With Threads Optimally: ");
            break;
        case replicatedThreads:
            printf("\n---------- Replicated With Threads %s: ", checkingEachModuleTime? " check improved" : "");
            break;
        case replicatedHT:
            printf("\n----------Replicated Wit Hyper-Threading %s: ", checkingEachModuleTime? " check improved" : "");
            break;
    }

    printf("%f \n\n", result / 5);

}