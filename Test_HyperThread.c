//
// Created by diego on 26/07/17.
//

#include "Test_HyperThread.h"

__thread int _thread_id;
int sharedValue = 0;

void* PingPong(void * arg){
    _thread_id = (int) (int64_t) arg;
    int i = 0;
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(_thread_id, &cpuset);

    if(THREAD_UTILS_NUM_THREADS > 1) {
        /* pin the thread to a core */
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
            fprintf(stderr, "Thread pinning failed!\n");
            exit(1);
        }
    }

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


    err = pthread_create(THREAD_UTILS_Threads[1], NULL, _start_routine, (void *) (int64_t) (useHyperThread) ? 2 : 1);

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