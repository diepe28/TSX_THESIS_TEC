#include <stdio.h>
#include "ThreadUtils.h"

pthread_t ** THREAD_UTILS_Threads;
int THREAD_UTILS_NUM_THREADS;

/// Starts with pthread_create each thread in THREAD_UTILS_Threads, and performs a pthread_join on each one.
/// \param _start_routine the function each thread executed
/// \param _thread_routine_args arguments to the _start_routine
void THREAD_UTILS_StartThreads(void * _start_routine, void * _thread_routine_args) {

    int j, err;

    // Creates THREAD_UTILS_NUM_THREADS-1 threads, starts at 1 because of the main thread
    for (j = 1; j < THREAD_UTILS_NUM_THREADS; j++)
        err = pthread_create(THREAD_UTILS_Threads[j], NULL, _start_routine, (void*)(int64_t)j);

    // Executes the routine in main thread
    ((void (*)(void)) _start_routine)();

    // Waits for the THREAD_UTILS_NUM_THREADS other threads
    for (j = 1; j < THREAD_UTILS_NUM_THREADS; j++)
        pthread_join(*THREAD_UTILS_Threads[j], NULL);
}

/// Returns the range on which each thread must work on.
/// \param threadIndex the index of the current thread in THREAD_UTILS_Threads
/// \param endIndex an out parameter that will hold the end of the range
/// \param numValues the number of values to process work array
/// \param numThreads the number of working threads
/// \return start index of the range to work.
int THREAD_UTILS_GetRangeFromThreadId(int threadIndex, int* endIndex, int numValues, int numThreads){
    int valuesPerThread = numValues / numThreads;
    int startIndex;
    startIndex = threadIndex * valuesPerThread;
    *endIndex = (startIndex + valuesPerThread);
    if (threadIndex == numThreads-1)
        *endIndex += (numValues % numThreads);
    return startIndex;
}

/// Gets using the threadId the index of such thread in THREAD_UTILS_Threads
/// \param threadId the id of the given thread
/// \return the index of the thread in THREAD_UTILS_Threads
int THREAD_UTILS_GetThreadIndex(pthread_t threadId) {
    int i;
    for (i = 0; i < THREAD_UTILS_NUM_THREADS; i++) {
        if (pthread_equal(threadId, *THREAD_UTILS_Threads[i]))
            return i;
    }
    return 0;
}

/// Initialize the THREAD_UTILS_Threads array with THREAD_UTILS_NUM_THREADS dimensions
void THREAD_UTILS_CreateThreads(){
    int i = 1;

    THREAD_UTILS_Threads = (pthread_t **) malloc(sizeof(pthread_t*) * THREAD_UTILS_NUM_THREADS);

    // main pthread id
    pthread_t mainThread = pthread_self();
    THREAD_UTILS_Threads[0] = &mainThread;

    for (; i < THREAD_UTILS_NUM_THREADS; i++)
        THREAD_UTILS_Threads[i] = (pthread_t*) malloc(sizeof(pthread_t));

}

/// Frees the memory of each thread in THREAD_UTILS_Threads and the array itself
void THREAD_UTILS_DestroyThreads(){
    int i = 1;

    for (; i < THREAD_UTILS_NUM_THREADS; i++)
        free(THREAD_UTILS_Threads[i]);

    free(THREAD_UTILS_Threads);
}

/// Returns the number of threads allocated
int THREAD_UTILS_GetNumThreads(){
    return THREAD_UTILS_NUM_THREADS;
}

/// Sets the number of thread to be created and used. Must be called before THREAD_UTILS_CreateThreads
/// \param n the new number of threads
void THREAD_UTILS_SetNumThreads(int n){
    THREAD_UTILS_NUM_THREADS = n;
}