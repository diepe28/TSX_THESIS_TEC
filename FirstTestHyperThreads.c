//
// Created by diego on 26/07/17.
//

#include "FirstTestHyperThreads.h"
#include "Queues.h"

#define DATATYPE long

__thread int _my_thread_id;
int sharedValue = 0;

//////////////////////// QUEUE TESTS ////////////////////////////
//////////////////////// QUEUE TESTS ////////////////////////////
extern SectionQueue sectionQueue;
extern SimpleQueue simpleQueue;
extern SimpleSyncQueue simpleSyncQueue;
extern lynxQ_t lynxQ1;

extern CheckFrequency checkFrequency;
extern QueueType queueType;

extern volatile long producerCount;
extern volatile long consumerCount;

void* PingPong(void * arg){
    _my_thread_id = (int) (int64_t) arg;
    int i = 0;
    SetThreadAffinity(_my_thread_id);

    for(; i < 100000000; i++){

        while(sharedValue == _my_thread_id);
        sharedValue = _my_thread_id;
    }

}

void HyperThreads_PingPongTest(int useHyperThread) {
//    int i, err, numThreads;
//    void *_start_routine = &PingPong;
//
//    numThreads = 2;
//    THREAD_UTILS_SetNumThreads(numThreads);
//    THREAD_UTILS_CreateThreads();
//
//    GTimer *timer = g_timer_new();
//
//    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
//                         (void*) (int64_t) ((useHyperThread) ? 2 : 1));
//
//    if (err) {
//        fprintf(stderr, "Failed to create thread\n");
//        exit(1);
//    }
//
//    PingPong(0);
//
//    // Waits for the THREAD_UTILS_NUM_THREADS other threads
//    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
//        pthread_join(*THREAD_UTILS_Threads[i], NULL);
//
//    g_timer_stop(timer);
//    gulong fractional_part = 0;
//    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
//    g_timer_destroy(timer);
//
//    THREAD_UTILS_DestroyThreads();
//
//    printf("Total number of seconds %f\n", seconds_elapsed);
}

int dummyFunc(long value){
    return value +1;
}

long CalcFunction(long index, long value){
    return index % 2 ?
           sqrt(3 * (value +1)) * 2 :
           - sqrt(3 * (value +1)) * 2;
    //- (value * 3 + 5) * 7 : (value * 3 + 5) * 7;
}

/////////////////// Heuristics ///////////////////
volatile long consumerResult = 0;
volatile long producerResult = 0;

void TestQueue_ThreadProducerOptimal(void * arg) {
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, result = 0;

    SetThreadAffinity(_my_thread_id);

    for (i = 0; i < NUM_VALUES; i++) {
        auxValue = CalcFunction(i, values[i]);
        dummyFunc(auxValue);
        result += auxValue;
        //printf("Producer local result %ld\n", result);
    }
    producerResult = result;
}

void TestQueue_ThreadConsumerOptimal(void * arg) {
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, result = 0, auxValue, otherValue;

    SetThreadAffinity(_my_thread_id);

    for (i = 0; i < NUM_VALUES; i++) {
        auxValue = CalcFunction(i, values[i]);
        otherValue = dummyFunc(auxValue);

        if (auxValue != --otherValue) {
            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i,
                   values[i], auxValue, otherValue);
            exit(1);
        }

        result += otherValue;
    }

    consumerResult = result;
}

double HyperThreads_QueueTestReplicatedOptimally() {
//    int err, numThreads;
//    void *_start_routine = &TestQueue_ThreadConsumerOptimal;
//    long i;
//
//    // random values
//    for (i = 0; i < NUM_VALUES; i++) {
//        values[i] = rand() % 1000000;
//    }
//
//    bool useHyperThread = false;
//    numThreads = 2;
//
//    THREAD_UTILS_SetNumThreads(numThreads);
//    THREAD_UTILS_CreateThreads();
//
//    GTimer *timer = g_timer_new();
//
//    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
//                         (void *) (int64_t) ((useHyperThread) ? 2 : 1));
//
//    if (err) {
//        fprintf(stderr, "Failed to create thread %d\n", 1);
//        exit(1);
//    }
//
//
//    TestQueue_ThreadProducerOptimal(0);
//
//    // Waits for the THREAD_UTILS_NUM_THREADS other threads
//    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
//        pthread_join(*THREAD_UTILS_Threads[i], NULL);
//
//    g_timer_stop(timer);
//    gulong fractional_part = 0;
//    gdouble milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
//    g_timer_destroy(timer);
//
//    THREAD_UTILS_DestroyThreads();
//
//    if(producerResult == consumerResult)
//        printf("Total Result: %ld--- number of milliseconds %f\n", producerResult, milliseconds_elapsed);
//    return milliseconds_elapsed;
}

double HyperThreads_SameThreadReplicated(){
//    long i, v1, v2,  result = 0;
//
//    // random values
//    for (i = 0; i < NUM_VALUES; i++) {
//        values[i] = rand() % 1000000;
//    }
//
//    GTimer *timer = g_timer_new();
//
//    for (i = 0; i < NUM_VALUES; i++){
//        v1 = CalcFunction(i, values[i]);
//        v2 = CalcFunction(i, values[i]);
//
//        if(v1 != v2){
//            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld, the value is %d !!!! %ld vs %ld \n\n\n\n", i, values[i], v1, v2);
//            exit(1);
//        }
//
//        result += v1;
//    }
//
//    producerResult = result;
//
//    g_timer_stop(timer);
//    gulong fractional_part = 0;
//    gdouble milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
//    g_timer_destroy(timer);
//
//    printf("Total Result: %ld --- number of milliseconds %f \n", producerResult, milliseconds_elapsed );
//    return milliseconds_elapsed ;
}

double HyperThreads_QueueTestNotReplicated() {
//    long i, auxValue, result = 0;
//
//    // random values
//    for (i = 0; i < NUM_VALUES; i++) {
//        values[i] = rand() % 1000000;
//    }
//
//    GTimer *timer = g_timer_new();
//
//    for (i = 0; i < NUM_VALUES; i++){
//        auxValue = CalcFunction(i, values[i]);
//        result += auxValue;
//    }
//
//    producerResult = result;
//
//    g_timer_stop(timer);
//    gulong fractional_part = 0;
//    gdouble milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
//    g_timer_destroy(timer);
//
//    printf("Total Result: %ld --- number of milliseconds %f \n", producerResult, milliseconds_elapsed);
//    return milliseconds_elapsed;
}

/// The next 3 pairs of methods are the same except the queue they use. It could have been a generic method that makes
/// the choice of which queue to use but that would be more complex and probably add ifs and elses..

///// Lynx Queue /////

void TestLynxQueue_Producer(void *arg) {
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int residue = 0, j, groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                queue_push_long(lynxQ1, calcValue);
            }
            queue_push_done(lynxQ1);
            break;

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValues[residue] = CalcFunction(i, values[i]);
                result += auxValues[residue];

                if (residue++ == groupLimit) {
                    for (j = 0; j < MODULO; j++) {
                        queue_push_long(lynxQ1, auxValues[j]);
                    }
                    residue = 0;
                }
            }
            queue_push_done(lynxQ1);
            break;

        case CheckFrequency_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (residue++ == groupLimit) {
                    queue_push_long(lynxQ1, xorMine);
                    xorMine = residue = 0;
                }
            }
            queue_push_done(lynxQ1);
            break;
    }

    producerResult = result;
}

void TestLynxQueue_Consumer(void *arg){
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    int residue = 0, j, groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    /* Busy wait until pop thread is ready to start */
    queue_busy_wait_pop_ready(lynxQ1);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                //otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error

                if (auxValue != otherValue) {
                    printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                    exit(1);
                }
                result += otherValue;
            }
            queue_pop_done(lynxQ1);
            break;

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValues[residue] = CalcFunction(i, values[i]);

                if (residue++ == groupLimit) {
                    for (j = 0; j < MODULO; j++) {
                        //otherValue = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error
                        if (auxValues[j] != otherValue) {
                            printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                            exit(1);
                        }
                        result += otherValue;
                    }
                    queue_pop_done(lynxQ1);
                    residue = 0;
                }
            }
            break;

        case CheckFrequency_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                xorResultMine ^= auxValue;

                result += auxValue;

                if (residue++ == groupLimit) {
                    //xorResultT1 = queue_pop_long(lynxQ1); // unless there are optimization this causes a compile error
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = residue = 0;
                }
            }
            queue_pop_done(lynxQ1);
            break;
    }

    consumerResult = result;
}

///// Simple Queue /////
void TestSimpleQueue_Producer(void *arg) {
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int residue = 0, j, groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                SimpleQueue_Enqueue(&simpleQueue, calcValue);
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValues[residue] = CalcFunction(i, values[i]);
                result += auxValues[residue];

                if (residue++ == groupLimit) {
                    for (j = 0; j < MODULO; j++) {
                        SimpleQueue_Enqueue(&simpleQueue, auxValues[j]);
                    }
                    residue = 0;
                }
            }
            break;

        case CheckFrequency_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (residue++ == groupLimit) {
                    SimpleQueue_Enqueue(&simpleQueue, xorMine);
                    xorMine = residue = 0;
                }
            }
            break;
    }

    producerResult = result;
}

void TestSimpleQueue_Consumer(void *arg){
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    long modulo = 0, j;
    int groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
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

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);

                if (modulo == groupLimit) {
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

        case CheckFrequency_Encoding:

            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                xorResultMine ^= auxValue;
                result += auxValue;

                if (modulo++ == groupLimit) {
                    xorResultT1 = SimpleQueue_Dequeue(&simpleQueue);
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = modulo = 0;
                }
            }
            break;
    }

    consumerResult = result;
}

///// Section Queue /////

void TestSectionQueue_Producer(void *arg) {
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, calcValue, result = 0, xorMine = 0;
    long auxValues[MODULO];
    int residue = 0, j, groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    switch (checkFrequency) {
        case CheckFrequency_everyTime:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                SectionQueue_Enqueue(&sectionQueue, calcValue);
            }
            break;

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                auxValues[residue] = CalcFunction(i, values[i]);
                result += auxValues[residue];

                if (residue++ == groupLimit) {
                    for (j = 0; j < MODULO; j++) {
                        SectionQueue_Enqueue(&sectionQueue, auxValues[j]);
                    }
                    residue = 0;
                }
            }
            break;

        case CheckFrequency_Encoding:
            for (i = 0; i < NUM_VALUES; i++) {
                calcValue = CalcFunction(i, values[i]);
                result += calcValue;
                xorMine ^= calcValue;

                if (residue++ == groupLimit) {
                    SectionQueue_Enqueue(&sectionQueue, xorMine);
                    xorMine = residue = 0;
                }
            }
            break;
    }

    producerResult = result;
}

void TestSectionQueue_Consumer(void *arg){
    _my_thread_id = (int) (int64_t) arg;
    long i = 0, auxValue, otherValue, result = 0;
    long auxValues[MODULO], xorResultMine = 0, xorResultT1;
    long modulo = 0, j;
    int groupLimit = MODULO -1;

    SetThreadAffinity(_my_thread_id);

    switch(checkFrequency) {
        case CheckFrequency_everyTime:
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

        case CheckFrequency_eachModuloTimes:
            for (i = 0; i < NUM_VALUES; i++) {
                modulo = i % MODULO;
                auxValues[modulo] = CalcFunction(i, values[i]);

                if (modulo == groupLimit) {
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

        case CheckFrequency_Encoding:

            for (i = 0; i < NUM_VALUES; i++) {
                auxValue = CalcFunction(i, values[i]);
                xorResultMine ^= auxValue;
                result += auxValue;

                if (modulo++ == groupLimit) {
                    xorResultT1 = SectionQueue_Dequeue(&sectionQueue);
                    if (xorResultT1 != xorResultMine) {
                        printf("\n\n\n\n AN ERROR WAS FOUND in iteration %ld \n\n\n", i);
                        exit(1);
                    }
                    xorResultMine = modulo = 0;
                }
            }
            break;
    }

    consumerResult = result;
}

double HyperThreads_QueueTestReplicated(int useHyperThread) {
//    int err, numThreads;
//    void *_start_routine;
//    long i, correctResult = 0;
//
//    // random values
//    for (i = 0; i < NUM_VALUES; i++) {
//        values[i] = rand() % 1000000;
//        correctResult += CalcFunction(i, values[i]);
//    }
//
//    consumerCount = producerCount = 0;
//
//    switch (queueType) {
//        case QueueType_Simple:
//            simpleQueue = SimpleQueue_Init();
//            _start_routine = &TestSimpleQueue_Consumer;
//            break;
//
//        case QueueType_Section:
//            sectionQueue = SectionQueue_Init();
//            _start_routine = &TestSectionQueue_Consumer;
//            break;
//
//        case QueueType_lynxq:
//            lynxQ1 = queue_init(LYNXQ_QUEUE_SIZE);
//            _start_routine = &TestLynxQueue_Consumer;
//            break;
//    }
//
//    GTimer *timer = g_timer_new();
//
//    numThreads = 2;
//
//    THREAD_UTILS_SetNumThreads(numThreads);
//    THREAD_UTILS_CreateThreads();
//
//    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
//                         (void *) (int64_t) ((useHyperThread) ? 2 : 1));
//
//    if (err) {
//        fprintf(stderr, "Failed to create thread %d\n", 1);
//        exit(1);
//    }
//
//    switch (queueType){
//        case QueueType_Simple:
//            TestSimpleQueue_Producer(0);
//            break;
//        case QueueType_Section:
//            TestSectionQueue_Producer(0);
//            break;
//        case QueueType_lynxq:
//            TestLynxQueue_Producer(0);
//            break;
//    }
//
//    // Waits for the THREAD_UTILS_NUM_THREADS other threads
//    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
//        pthread_join(*THREAD_UTILS_Threads[i], NULL);
//
//    g_timer_stop(timer);
//    gulong fractional_part = 0;
//    gdouble milliseconds_elapsed = g_timer_elapsed(timer, &fractional_part) * 1000;
//    g_timer_destroy(timer);
//
//    SimpleSyncQueue_Destroy(&simpleSyncQueue);
//
//    THREAD_UTILS_DestroyThreads();
//    if(lynxQ1) {
//        lynx_queue_print_numbers();
//        queue_finalize(lynxQ1);
//        free(lynxQ1);
//        lynxQ1 = NULL;
//    }
//
//    if(producerResult == consumerResult && producerResult == correctResult)
//        printf("Total Result: %ld --- produced %ld consumed %ld --- number of milliseconds %f\n", producerResult , producerCount, consumerCount, milliseconds_elapsed);
//    else
//        printf("Results are different, something happened... %ld vs %ld vs %ld \n", producerResult, consumerResult, correctResult);
//    return milliseconds_elapsed;
}

void HyperThreads_QueueTest(ExecMode execMode){
    int i = 0;
    double mean = 0, sd = 0;
    double times[NUM_RUNS] = {0};
    char queueTypeStr[30];
    char checkFrequencyStr[50];

//    if(!areValuesCalculated) {
//        // random values
//        for (i = 0; i < NUM_VALUES; i++) {
//            values[i] = rand() % 1000000;
//        }
//        areValuesCalculated = true;
//    }

    for (i=0; i < NUM_RUNS; i++) {
        switch (execMode) {
            case ExeMode_notReplicated:
                times[i] += HyperThreads_QueueTestNotReplicated();
                break;
            case ExeMode_replicatedSameThread:
                times[i] += HyperThreads_SameThreadReplicated();
                break;
            case ExeMode_replicatedThreadsOptimally:
                times[i] += HyperThreads_QueueTestReplicatedOptimally();
                break;
            case ExeMode_replicatedThreads:
                times[i] += HyperThreads_QueueTestReplicated(0);
                break;
            case ExeMode_replicatedHyperThreads:
                times[i] += HyperThreads_QueueTestReplicated(1);
                break;
        }
        mean += times[i];
    }

    mean /= NUM_RUNS;
    for (i= 0; i < NUM_RUNS; i++) {
        sd += fabs(mean - times[i]);
    }

    sd /= NUM_RUNS;

    switch (queueType){
        case QueueType_Simple :
            strcpy(queueTypeStr, "using SIMPLE queue,");
            break;
        case QueueType_Section :
            strcpy(queueTypeStr, "using SECTION queue");
            break;
        case QueueType_lynxq :
            strcpy(queueTypeStr, "using LYNXQ queue");
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

    switch (execMode) {
        case ExeMode_notReplicated:
            printf("\n---------- Not replicated: ");
            break;
        case ExeMode_replicatedSameThread:
            printf("\n---------- Replicated in the same thread: ");
            break;
        case ExeMode_replicatedThreadsOptimally:
            printf("\n---------- Replicated With Threads Optimally: ");
            break;
        case ExeMode_replicatedThreads:
            printf("\n---------- Replicated With Threads %s %s: ", queueTypeStr, checkFrequencyStr);
            break;
        case ExeMode_replicatedHyperThreads:
            printf("\n---------- Replicated With Hyper-Threads %s %s: ", queueTypeStr, checkFrequencyStr);
            break;
    }

    printf("Mean: %f SD: %f\n\n", mean, sd);

}

void HyperThreads_TestAllCombinations(){
    // Heuristics
//    HyperThreads_QueueTest(ExeMode_notReplicated);
//    HyperThreads_QueueTest(ExeMode_replicatedSameThread);
//    HyperThreads_QueueTest(ExeMode_replicatedThreadsOptimally);
//
//    printf("\n --- Simple Queue Size: %d ---\n\n", SIMPLE_QUEUE_MAX_ELEMENTS);
//
//    // Every Time, Simple Queue
//    Global_SetCheckFrequency(CheckFrequency_everyTime);
//    Global_SetQueueType(QueueType_Simple);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
//
//    // Every Module Times, Simple Queue
//    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
//
//    // Every Modules Times With Encoding, Simple Queue
//    Global_SetCheckFrequency(CheckFrequency_Encoding);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
//
//    //////////////////////////////////////////////////////////
//    //////////////////////////////////////////////////////////
//    printf("\n --- Section Queue Info: NumSections: %d  SectionSize: %d ---\n\n", NUM_SECTIONS, SECTION_SIZE);
//    // Every Time, Section Queue
//    Global_SetCheckFrequency(CheckFrequency_everyTime);
//    Global_SetQueueType(QueueType_Section);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
//
//    // Every Module Times, Section Queue
//    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
//
//    // Every Modules Times With Encoding, Section Queue
//    Global_SetCheckFrequency(CheckFrequency_Encoding);
//    HyperThreads_QueueTest(ExeMode_replicatedThreads);
//    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    printf("\n --- LYNX Queue --- \n");
    // Every Time, lynx Queue
    Global_SetCheckFrequency(CheckFrequency_everyTime);
    Global_SetQueueType(QueueType_lynxq);
    HyperThreads_QueueTest(ExeMode_replicatedThreads);
    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);

    // Every Module Times, lynx Queue
    Global_SetCheckFrequency(CheckFrequency_eachModuloTimes);
    HyperThreads_QueueTest(ExeMode_replicatedThreads);
    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);

    // Every Modules Times With Encoding, lynx Queue
    Global_SetCheckFrequency(CheckFrequency_Encoding);
    HyperThreads_QueueTest(ExeMode_replicatedThreads);
    HyperThreads_QueueTest(ExeMode_replicatedHyperThreads);
}