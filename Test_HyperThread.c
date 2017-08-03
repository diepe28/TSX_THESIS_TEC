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
SimpleQueue simpleQueue;
bool checkEveryTime = true;
bool useSectionQueue = false;

long CalcFunction(int index, long value){
    return index % 2 ?
           sqrt(3 * (value +1)) * 2 :
           - sqrt(3 * (value +1)) * 2;
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

double HyperThreads_QueueTestReplicatedOptimally() {
    int err, numThreads;
    void *_start_routine = &TestQueue_ThreadConsumerOptimal;
    long i;

    bool useHyperThread = true;
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

#define MODULO 10

void TestSectionQueue_Producer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, result = 0;

    SetThreadAffinity(_thread_id);

    ///////////// Producing every time
    if(checkEveryTime){
        for (i = 0; i < NUM_VALUES; i++) {
            auxValue = CalcFunction(i, values[i]);
            SectionQueue_Enqueue(&sectionQueue, auxValue);
            result += auxValue;
        }

    } else{ ///////////// Producing each MODULE times
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
    }
}

void TestSectionQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    ///////////// Syncing Every time
    if(checkEveryTime) {
        for (i = 0; i < NUM_VALUES; i++) {
            auxValue = CalcFunction(i, values[i]);
            otherValue = SectionQueue_Dequeue(&sectionQueue);

            if (auxValue != otherValue) {
                printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i,
                       values[i], auxValue, otherValue);
                exit(1);
            }

            result += otherValue;
        }
    } else {///////////// Consuming each MODULO times
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
    }

}

void TestSimpleQueue_Producer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, result = 0;

    SetThreadAffinity(_thread_id);

    ///////////// Producing every time
    if(checkEveryTime){
        for (i = 0; i < NUM_VALUES; i++) {
            auxValue = CalcFunction(i, values[i]);
            SimpleQueue_Enqueue(&simpleQueue, auxValue);
            result += auxValue;
        }
    } else{ ///////////// Producing each MODULE times
        long auxValues[MODULO];
        int modulo, j;
        for (i = 0; i < NUM_VALUES; i++){
            modulo = i % MODULO;

            auxValues[modulo] = CalcFunction(i, values[i]);
            result += auxValues[modulo];

            if (modulo == MODULO-1)
                for(j = 0; j < MODULO; j++)
                    SimpleQueue_Enqueue(&simpleQueue, auxValues[j]);
        }
    }
}

void TestSimpleQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], t1Values[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    ///////////// Syncing Every time
    if(checkEveryTime) {
        for (i = 0; i < NUM_VALUES; i++) {
            auxValue = CalcFunction(i, values[i]);
            otherValue = SimpleQueue_Dequeue(&simpleQueue);

            if (auxValue != otherValue) {
                printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i,
                       values[i], auxValue, otherValue);
                exit(1);
            }

            result += otherValue;
        }
    } else { ///////////// Consuming each MODULO times
        for (i = 0; i < NUM_VALUES; i++) {
            modulo = i % MODULO;
            auxValues[modulo] = CalcFunction(i, values[i]);

            if (modulo == MODULO - 1) {
//                for (j = 0; j < MODULO; j++) {
//                    otherValue = SimpleQueue_Dequeue(&simpleQueue);
//
//                    if (auxValues[j] != otherValue) {
//                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n",
//                               i,
//                               values[i], auxValue, otherValue);
//                        exit(1);
//                    }
//                    result += otherValue;
//                }
                for (j = 0; j < MODULO; j++) {
                    t1Values[j] = SimpleQueue_Dequeue(&simpleQueue);
                }
                if(auxValues[0] != t1Values[0] ||
                   auxValues[1] != t1Values[1] ||
                   auxValues[2] != t1Values[2] ||
                   auxValues[3] != t1Values[3] ||
                   auxValues[4] != t1Values[4]) {
                    printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n",
                           i, values[i], auxValue, otherValue);
                    exit(1);
                }
                result += auxValues[0] + auxValues[1] + auxValues[2] + auxValues[3] + auxValues[4];
            }
        }
    }

}

double HyperThreads_QueueTestReplicated(int useHyperThread) {
    int err, numThreads;
    void *_start_routine;
    long i;

    if(useSectionQueue){
        sectionQueue = SectionQueue_Init();
        _start_routine = &TestSectionQueue_Consumer;
    }else {
        simpleQueue = SimpleQueue_Init();
        _start_routine = &TestSimpleQueue_Consumer;
    }
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

    useSectionQueue? TestSectionQueue_Producer(0) : TestSimpleQueue_Producer(0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    THREAD_UTILS_DestroyThreads();

    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

void HyperThreads_QueueTest(ExecMode execMode){
    int i = 0, n = 5;
    double result = 0;

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
            printf("\n---------- Replicated With Threads %s %s: ",
                   checkEveryTime? "checking every time" : "check improved",
                   useSectionQueue? "using section queue" : "using simple queue");
            break;
        case replicatedHT:
            printf("\n----------Replicated With Hyper-Threading %s %s: ",
                   checkEveryTime? "checking every time" : "check improved",
                   useSectionQueue? "using section queue" : "using simple queue");
            break;
    }

    printf("%f \n\n", result / 5);

}

void HyperThreads_UseSectionQueueType(bool sectionQueue){
    useSectionQueue = sectionQueue;
}
void HyperThreads_CheckEveryTime(bool checkingEveryTime){
    checkEveryTime = checkingEveryTime;
}