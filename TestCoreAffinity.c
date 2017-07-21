
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


__thread int __thread_id;

int useHyperThread = 0;
static pthread_barrier_t barriersCore0[2], barriersCore1[2];
long long globalTempResults[2];
int hyperThreadsFinished[2] = {0};
void * thread_affinity_test(void* arg) {
    __thread_id = (int) (int64_t) arg;

    if(useHyperThread) {
        if (__thread_id == 1) {
            __thread_id = 2;
        }
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

    if(useHyperThread) {
        if (__thread_id == 2) {
            __thread_id = 1;
        }
    }
    startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id, &endIndex, NUM_VALUES, THREAD_UTILS_NUM_THREADS);
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

void CoreAffinity_View(int numThreads, int withHyperThread){

    int i, err;
    void * _start_routine = &thread_affinity_test;
    long long totalResult = 0;
    useHyperThread = withHyperThread;

    THREAD_UTILS_SetNumThreads(numThreads);
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
    thread_affinity_test((void*)0);

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    //gdouble milliseconds_elapsed = seconds_elapsed * 1000;
    g_timer_destroy(timer);

    for (i = 0; i < THREAD_UTILS_NUM_THREADS; i++)
        totalResult += results[i];

    free(results);
    THREAD_UTILS_DestroyThreads();

    printf("The final result is: %lld... total number of seconds %f\n", totalResult, seconds_elapsed);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void barrierInit(){
    int i = 0;
    for(i; i < 2; i++){
        pthread_barrier_init(&barriersCore0[i], NULL, 2);
        pthread_barrier_init(&barriersCore1[i], NULL, 2);
    }
}

void barrierWait(int index){
    // core 1
    if(__thread_id % 2){
        pthread_barrier_wait(&barriersCore0[index]);
    }else{// core 0
        pthread_barrier_wait(&barriersCore1[index]);
    }
}

long long int Affinity_CalcFunc(int i) {
    long long auxResult = values[i] * 4;

    int j = 0;
    while(j++ < 50){
            auxResult *= sqrt(auxResult);
            auxResult /= sqrt(auxResult/2);
            auxResult += sqrt(auxResult);
            auxResult -= sqrt(auxResult/2);
        }
    return (i % 2)? fmod(auxResult, 100) : -fmod(auxResult, 100);
}

void affinity_setup(){
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(__thread_id, &cpuset);

    if(THREAD_UTILS_NUM_THREADS > 1) {
        /* pin the thread to a core */
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
            fprintf(stderr, "Thread pinning failed!\n");
            exit(1);
        }
    }
}

void thread_task_normal(void* arg) {
    int i, startIndex, endIndex;
    long long tempResult = 0;

    __thread_id = (int) (int64_t) arg;
    affinity_setup();
    startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id, &endIndex, NUM_VALUES, THREAD_UTILS_NUM_THREADS);

    for (i = startIndex; i < endIndex; i++) {
        tempResult += Affinity_CalcFunc(i);
    }

    results[__thread_id] = tempResult;
    printf("Thread[%d] finished, its local result is %lld \n", __thread_id, tempResult);
}

void thread_task_normally_replicated(void* arg) {
    int i, startIndex, endIndex;
    long long tempResult = 0, v1, v2;

    __thread_id = (int) (int64_t) arg;
    affinity_setup();
    startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id, &endIndex, NUM_VALUES, THREAD_UTILS_NUM_THREADS);

    for (i = startIndex; i < endIndex; i++) {
        v1 = Affinity_CalcFunc(i);
        v2 = Affinity_CalcFunc(i);
        if(v1 == v2) {
            tempResult += v1;
        }else{
            // do something
        }
    }

    results[__thread_id] = tempResult;
    printf("Thread[%d] finished, its local result is %lld \n", __thread_id, tempResult);
}

void thread_task_hyper_replicated(void* arg) {
    int i, startIndex, endIndex, isHyperThread;
    long long tempResult = 0, localTempResult, instructionsWasted = 0;

    __thread_id = (int) (int64_t) arg;
    isHyperThread = __thread_id == 2 || __thread_id == 3;
    affinity_setup();

    if(isHyperThread) {
        // must iterate through the same range as its partner
        startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id-2, &endIndex, NUM_VALUES,
                                                       THREAD_UTILS_NUM_THREADS / 2);
    }else{
        startIndex = THREAD_UTILS_GetRangeFromThreadId(__thread_id, &endIndex, NUM_VALUES,
                                                       THREAD_UTILS_NUM_THREADS / 2);
    }

    barrierWait(0);

    for (i = startIndex; i < endIndex; i++) {

        if(isHyperThread){
            globalTempResults[__thread_id-2] = Affinity_CalcFunc(i);
            //finished and waiting for main thread
            hyperThreadsFinished[__thread_id-2] = 1; // with barriers this is not necessary...
            barrierWait(0);
            //barrierWait(1); //barrier solution
        }else {
            localTempResult = Affinity_CalcFunc(i);

            //finished and waiting for hyper-thread
            while(hyperThreadsFinished[__thread_id] == 0);

            //barrierWait(0); //barrier solution

            if (localTempResult == globalTempResults[__thread_id]) {
                tempResult += localTempResult;

                // for next iteration
                hyperThreadsFinished[__thread_id] = 0; // again, with barriers this is not necessary...

            } else {
                // do something, restore to checkpoint I guess
            }
            //barrierWait(1); //barrier solution

            barrierWait(0);
        }
    }

    results[__thread_id] = tempResult;
    printf("Thread[%d] finished, its local result is %lld , instructions wasted: %lld\n", __thread_id, tempResult, instructionsWasted);
}


void CoreAffinity_Replication_Test(ExecutionType executionType){

    int i, err, numThreads;
    void * _start_routine;
    double totalResult = 0;

    // random values
    for(i = 0; i < NUM_VALUES; i++){
        values[i] = rand() % 1000000;
    }

    useHyperThread = 0;
    switch (executionType){
        case normal:
            numThreads = 1;
            _start_routine = &thread_task_normal;
            break;
        case normallyReplicated:
            numThreads = 2;
            _start_routine = &thread_task_normally_replicated;
            break;
        case normallyReplicatedWithHT:
            numThreads = 4;
            _start_routine = &thread_task_normally_replicated;
            break;
        case hyperReplicated:
            numThreads = 4;
            barrierInit();
            _start_routine = &thread_task_hyper_replicated;
            break;
    }

    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();
    results = (malloc(sizeof(double) * THREAD_UTILS_GetNumThreads()));

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
    switch (executionType){
        case normal:
            thread_task_normal(0);
            break;
        case normallyReplicated:
            thread_task_normally_replicated(0);
            break;
        case normallyReplicatedWithHT:
            thread_task_normally_replicated(0);
            break;
        case hyperReplicated:
            thread_task_hyper_replicated(0);
            break;
    }

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (i = 1; i < THREAD_UTILS_NUM_THREADS; i++)
        pthread_join(*THREAD_UTILS_Threads[i], NULL);

    g_timer_stop(timer);
    gulong fractional_part = 0;
    gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
    //gdouble milliseconds_elapsed = seconds_elapsed * 1000;
    g_timer_destroy(timer);

    for (i = 0; i < THREAD_UTILS_NUM_THREADS; i++)
        totalResult += results[i];

    free(results);
    THREAD_UTILS_DestroyThreads();

    printf("The final result is: %f... total number of seconds %f\n", totalResult, seconds_elapsed);
}