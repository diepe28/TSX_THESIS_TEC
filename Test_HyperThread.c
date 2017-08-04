//
// Created by diego on 26/07/17.
//

#include "Test_HyperThread.h"
#define DATATYPE long

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
lynxQ_t lynxQ1;

CheckFrequency checkFrequency = everyTime;
QueueType queueType = simpleQueueType;

long CalcFunction(int index, long value){
    return index % 2 ?
           sqrt(3 * (value +1)) * 2 :
           - sqrt(3 * (value +1)) * 2;
}

/////////////////// Heuristics ///////////////////

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

#define MODULO 5

void TestLynxQueue_Producer(void *arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                SimpleQueue_Enqueue(&simpleQueue, calcValue);
                queue_push_long(lynxQ1, calcValue);
                queue_push_done(lynxQ1);
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                result += auxValues[modulo];

                if (modulo == MODULO - 1)
                    for (j = 0; j < MODULO; j++) {
                        queue_push_long(lynxQ1, auxValues[j]);
                        queue_push_done(lynxQ1);
                    }
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (i % MODULO == MODULO - 1) {
                    queue_push_long(lynxQ1, xorMine);
                    queue_push_done(lynxQ1);
                    xorMine = 0;
                }
            }
            break;
    }
}

void TestLynxQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    int modulo, j;

    SetThreadAffinity(_thread_id);

    /* Busy wait until pop thread is ready to start */
    queue_busy_wait_pop_ready(lynxQ1);

    switch(checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                otherValue = queue_pop_long(lynxQ1);
                queue_pop_done(lynxQ1);

                if (auxValue != otherValue) {
                    printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                    exit(1);
                }
                result += otherValue;
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);

                if (modulo == MODULO - 1) {
                    for (j = 0; j < MODULO; j++) {
                        otherValue = queue_pop_long(lynxQ1);
                        queue_pop_done(lynxQ1);

                        if (auxValues[j] != otherValue) {
                            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                            exit(1);
                        }
                        result += otherValue;
                    }
                }
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                xorResultMine ^= auxValues[modulo];

                result += auxValues[modulo];

                if (modulo == MODULO - 1) {
                    xorResultT1 = queue_pop_long(lynxQ1);
                    queue_pop_done(lynxQ1);
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = 0;
                }
            }
            break;
    }
}

void TestSectionQueue_Producer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                SectionQueue_Enqueue(&sectionQueue, calcValue);
                result += calcValue;
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                result += auxValues[modulo];

                if (modulo == MODULO - 1)
                    for (j = 0; j < MODULO; j++)
                        SectionQueue_Enqueue(&sectionQueue, auxValues[j]);
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (i % MODULO == MODULO - 1) {
                    SectionQueue_Enqueue(&sectionQueue, xorMine);
                    xorMine = 0;
                }
            }
            break;
    }
}

void TestSectionQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    int modulo, j;

    SetThreadAffinity(_thread_id);

    switch(checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                otherValue = SectionQueue_Dequeue(&sectionQueue);

                if (auxValue != otherValue) {
                    printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                    exit(1);
                }
                result += otherValue;
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);

                if (modulo == MODULO - 1) {
                    for (j = 0; j < MODULO; j++) {
                        otherValue = SectionQueue_Dequeue(&sectionQueue);

                        if (auxValues[j] != otherValue) {
                            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                            exit(1);
                        }
                        result += otherValue;
                    }
                }
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                xorResultMine ^= auxValues[modulo];

                result += auxValues[modulo];

                if (modulo == MODULO - 1) {
                    xorResultT1 = SectionQueue_Dequeue(&sectionQueue);
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = 0;
                }
            }
            break;
    }

}

void TestSimpleQueue_Producer(void *arg) {
    _thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int modulo, j;

    SetThreadAffinity(_thread_id);

    switch (checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                SimpleQueue_Enqueue(&simpleQueue, calcValue);
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                result += auxValues[modulo];

                if (modulo == MODULO - 1)
                    for (j = 0; j < MODULO; j++)
                        SimpleQueue_Enqueue(&simpleQueue, auxValues[j]);
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (i % MODULO == MODULO - 1) {
                    SimpleQueue_Enqueue(&simpleQueue, xorMine);
                    xorMine = 0;
                }
            }
            break;
    }
}

void TestSimpleQueue_Consumer(void *arg){
    _thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    int modulo, j;

    SetThreadAffinity(_thread_id);

    switch(checkFrequency) {
        case everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                otherValue = SimpleQueue_Dequeue(&simpleQueue);

                if (auxValue != otherValue) {
                    printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                    exit(1);
                }
                result += otherValue;
            }
            break;

        case eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);

                if (modulo == MODULO - 1) {
                    for (j = 0; j < MODULO; j++) {
                        otherValue = SimpleQueue_Dequeue(&simpleQueue);

                        if (auxValues[j] != otherValue) {
                            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                            exit(1);
                        }
                        result += otherValue;
                    }
                }
            }
            break;

        case eachModuloTimes_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);
                xorResultMine ^= auxValues[modulo];

                result += auxValues[modulo];

                if (modulo == MODULO - 1) {
                    xorResultT1 = SimpleQueue_Dequeue(&simpleQueue);
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = 0;
                }
            }
            break;
    }
}

double HyperThreads_QueueTestReplicated(int useHyperThread) {
    int err, numThreads;
    void *_start_routine;
    long i;

    switch (queueType) {
        case simpleQueueType:
            simpleQueue = SimpleQueue_Init();
            _start_routine = &TestSimpleQueue_Consumer;
            break;

        case sectionQueueType:
            sectionQueue = SectionQueue_Init();
            _start_routine = &TestSectionQueue_Consumer;
            break;

        case lynxqQueueType:
            lynxQ1 = queue_init(LYNXQ_QUEUE_SIZE);
            _start_routine = &TestLynxQueue_Consumer;
            break;
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

    switch (queueType){
        case simpleQueueType:
            TestSimpleQueue_Producer(0);
            break;
        case sectionQueueType:
            TestSectionQueue_Producer(0);
            break;
        case lynxqQueueType:
            TestLynxQueue_Producer(0);
            break;
    }


    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    g_timer_destroy(timer);

    THREAD_UTILS_DestroyThreads();
    if(lynxQ1)
        free(lynxQ1);

    printf("Total number of seconds %f\n", seconds_elapsed);
    return seconds_elapsed;
}

void HyperThreads_QueueTest(ExecMode execMode){
    int i = 0, n = 5;
    double mean = 0;

    for (; i < n; i++) {
        switch (execMode) {
            case notReplicated:
                mean += HyperThreads_QueueTestNotReplicated();
                break;
            case replicatedSameThread:
                mean += HyperThreads_SameThreadReplicated();
                break;
            case replicatedThreadsOptimally:
                mean += HyperThreads_QueueTestReplicatedOptimally();
                break;
            case replicatedThreads:
                mean += HyperThreads_QueueTestReplicated(0);
                break;
            case replicatedHT:
                mean += HyperThreads_QueueTestReplicated(1);
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
                   checkFrequency == everyTime? "checking every time" :
                   checkFrequency == eachModuloTimes? "each modulo times" :
                    "each module times with encoding",
                   queueType == sectionQueueType? "using section queue" :
                   queueType == simpleQueueType? "using simple queue" :
                   "using lynxq queue");
            break;
        case replicatedHT:
            printf("\n----------Replicated With Hyper-Threading %s %s: ",
                   checkFrequency == everyTime? "checking every time" :
                   checkFrequency == eachModuloTimes? "each modulo times" :
                   "each module times with encoding",
                   queueType == sectionQueueType? "using section queue" :
                   queueType == simpleQueueType? "using simple queue" :
                   "using lynxq queue");
            break;
    }

    printf("%f \n\n", mean / n);

}

void HyperThreads_SetQueueType(QueueType newQueueType){
    queueType = newQueueType;
}
void HyperThreads_SetCheckFrequency(CheckFrequency newCheckFrequency){
    checkFrequency = newCheckFrequency;
}