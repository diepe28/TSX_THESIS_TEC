//
// Created by diego on 02/08/17.
//

#include "Test_Queue.h"

SimpleQueue q1;

void TestQueue_ThreadProducer(void * arg){
    int _thread_id = (int) (int64_t) arg;
    int i = 0, auxValue;
    for (i = 0; i < NUM_VALUES; i++){
        auxValue = values[i] * 2;
        SimpleQueue_Enqueue(&q1, auxValue);
    }

}

void TestQueue_ThreadConsumer(void * arg){
    int _thread_id = (int) (int64_t) arg;
    int i = 0, auxValue, otherValue;
    for (i = 0; i < NUM_VALUES; i++){
        auxValue = values[i] * 2;
        otherValue = SimpleQueue_Dequeue(&q1);
        if(auxValue != otherValue){
            printf("AN ERROR WAS FOUND!!!!\n\n\n\n");
        }
    }
}

void TestQueue_SimpleTest(int useHyperThread){
    int i, err, numThreads;
    void *_start_routine = &TestQueue_ThreadConsumer;

    q1 = SimpleQueue_Init();
    numThreads = 2;

    // random values
    for(i = 0; i < NUM_VALUES; i++){
        values[i] = rand() % 1000000;
    }

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();

    GTimer *timer = g_timer_new();

    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine,
                         (void *) (int64_t) (useHyperThread) ? 2 : 1);

    if (err) {
        fprintf(stderr, "Failed to create thread %d\n", i);
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

    printf("Total number of seconds %f\n", seconds_elapsed);
}