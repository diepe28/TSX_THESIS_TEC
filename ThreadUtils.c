#include <stdio.h>
#include "ThreadUtils.h"

pthread_t ** THREAD_UTILS_Threads;
int THREAD_UTILS_NUM_THREADS;

void THREAD_UTILS_StartThreads(void * _start_routine, void * _thread_routine_args) {

    int j, err;

    for (j = 0; j < THREAD_UTILS_NUM_THREADS; j++)
        err = pthread_create(THREAD_UTILS_Threads[j], NULL, _start_routine, _thread_routine_args);
    for (j = 0; j < THREAD_UTILS_NUM_THREADS; j++)
        pthread_join(*THREAD_UTILS_Threads[j], NULL);
}

int THREAD_UTILS_GetRangeFromThreadId(int threadIndex, int* endIndex, int numValues){
    int valuesPerThread = numValues / THREAD_UTILS_NUM_THREADS;
    int startIndex;
    startIndex = threadIndex * valuesPerThread;
    *endIndex = (startIndex + valuesPerThread);
    if (threadIndex == THREAD_UTILS_NUM_THREADS-1)
        *endIndex += (numValues % THREAD_UTILS_NUM_THREADS);
    return startIndex;
}

int THREAD_UTILS_GetThreadIndex(pthread_t threadId) {
    int i;
    for (i = 0; i < THREAD_UTILS_NUM_THREADS; i++) {
        if (pthread_equal(threadId, *THREAD_UTILS_Threads[i]))
            return i;
    }
    return 0;
}

void THREAD_UTILS_CreateThreads(){
    int i = 0;

    THREAD_UTILS_Threads = (pthread_t **) malloc(sizeof(pthread_t*) * THREAD_UTILS_NUM_THREADS);

    for (; i < THREAD_UTILS_NUM_THREADS; i++)
        THREAD_UTILS_Threads[i] = (pthread_t*) malloc(sizeof(pthread_t));

}

void THREAD_UTILS_DestroyThreads(){
    int i = 0;

    for (; i < THREAD_UTILS_NUM_THREADS; i++)
        free(THREAD_UTILS_Threads[i]);

    free(THREAD_UTILS_Threads);
}

int THREAD_UTILS_GetNumThreads(){
    return THREAD_UTILS_NUM_THREADS;
}

int THREAD_UTILS_SetNumThreads(int n){
    THREAD_UTILS_NUM_THREADS = n;
}