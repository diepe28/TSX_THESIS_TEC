
//
// Created by diego on 19/07/17.
//

#define _GNU_SOURCE

#include "TestCoreAffinity.h"
#include "ThreadUtils.h"
#include "TestTSX.h"
#include <sched.h>

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <glib.h>


int64_t count=0;
__thread int __thread_id;
volatile int main_work = 1;

static pthread_barrier_t init_barr;


void ALRMhandler (int sig)
{
    main_work = 0;
}

void * thread_main(void* arg)
{
    __thread_id = (int)(int64_t)arg;
    if(__thread_id == 1) {
        __thread_id = 2;
    }

    int i, j, startIndex, endIndex, isPrime = 0;
    double localResult = 0, auxResult;

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(__thread_id, &cpuset);

    /* pin the thread to a core */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        fprintf(stderr, "Thread pinning failed!\n");
        exit(1);
    }

    if(__thread_id == 2) {
        __thread_id = 1;
    }
    startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id, &endIndex, NUM_VALUES);
    for (i = startIndex; i < endIndex; i++) {
        isPrime = 0;

        for(j = 2; j < sqrt(values[i]); j++){
            if(values[i] % j == 0){
                isPrime = 1;
                break;
            }
        }

        if(isPrime){
            localResult++;
        }

    }

    results[__thread_id] = localResult;
    printf("Thread[%d] finished, it found %f prime numbers...\n", __thread_id, localResult);
}

void CoreAffinity_View(){

    int i, err;
    void * _start_routine = &thread_main;
    long long totalResult = 0;

    THREAD_UTILS_SetNumThreads(2);
    THREAD_UTILS_CreateThreads();
    results = (malloc(sizeof(double) * THREAD_UTILS_GetNumThreads()));

    // random values
    for(i = 0; i < NUM_VALUES; i++){
        values[i] = rand() % 10000;
        //values[i] = 2;
    }

    GTimer *timer = g_timer_new();

    // Creates THREAD_UTILS_NUM_THREADS-1 threads, starts at 1 because of the main thread
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++){
        err = pthread_create(THREAD_UTILS_Threads[i], NULL, _start_routine, (void*)(int64_t)i);
        if(err){
            fprintf(stderr, "Failed to create thread %d\n", i);
            exit(1);
        }
    }

    // Executes the routine in main thread
    thread_main((void*)0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    gdouble milliseconds_elapsed = seconds_elapsed * 1000;
    g_timer_destroy(timer);

    for (i = 0; i < THREAD_UTILS_NUM_THREADS; i++)
        totalResult += results[i];

    free(results);
    THREAD_UTILS_DestroyThreads();

    printf("The final result is: %lld... total number of seconds %f\n", totalResult, seconds_elapsed);
}
