#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <sys/types.h>
#include<pthread.h>
#include <stdlib.h>

extern pthread_t ** THREAD_UTILS_Threads;
extern int THREAD_UTILS_NUM_THREADS;

void THREAD_UTILS_StartThreads(void * _start_routine, void * _thread_routine_args);
int THREAD_UTILS_GetRangeFromThreadId(int threadIndex, int* endIndex, int numValues);
int THREAD_UTILS_GetThreadIndex(pthread_t threadId);
void THREAD_UTILS_CreateThreads();
void THREAD_UTILS_DestroyThreads();
int THREAD_UTILS_GetNumThreads();
void THREAD_UTILS_SetNumThreads(int n);

#endif //THREAD_UTILS_H
