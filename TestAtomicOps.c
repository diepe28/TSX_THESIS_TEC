//
// Created by diego on 19/07/17.
//

#include "TestAtomicOps.h"
#include "TestTSX.h"

/*Everything in this file was to test whether atomic operations inside hardware memory transactions caused aborts.
 * We now focus on how to replicate, and leave HTM for later (hopefully)
 * */

////////////////////////// TEST atomic operation with TSX
long numAborts = 0;
int sharedVariable = 0;
pthread_mutex_t abortMutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct{
    long v1,v2,v3,v4,v5,v6,v7,v8;
    //v9,v10,v11,v12,v13,v14,v15,v16;
    //v17,v18,v19,v20,v21,v22,v23,v24;
}Enum64Bits;

Enum64Bits e0, e1;
int myVar1 = 0, myVar2 = 0;
int hyperThreadFinished = 0;
pthread_barrier_t barrier1, barrier2;

void AtomicOps_IncAbort(){
    pthread_mutex_lock(&abortMutex);
    numAborts++;
    pthread_mutex_unlock(&abortMutex);
}

void AtomicOps_ThreadFunc1(void *args) {
    int i, transactionStatus, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());
    char *message = NULL;

    //have a barrier here

    for (i = 0; i < NUM_VALUES; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus == _XBEGIN_STARTED) {
            globalCount++;
            _xend();
        } else { //... non-transactional fallback path...
            AtomicOps_IncAbort();
//            message = TSX_GetTransactionAbortMessage(transactionStatus);
//            printf("%s ... Was explicit abort? %d "
//                           "Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//            free(message);
            //i--;
        }
    }

}

void AtomicOps_ThreadFunc2(void *args){
    int i, transactionStatus, c1 = 0, c2 = 0, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());
    char * message = NULL;

    //

    for (i = 0; i < NUM_VALUES; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus  == _XBEGIN_STARTED) {
            c1++;
            _xend();
        } else { //... non-transactional fallback path...
            AtomicOps_IncAbort();
//            message = TSX_GetTransactionAbortMessage(transactionStatus);
//            printf("%s ... Was explicit abort? %d "
//                           "Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//            free(message);
        }
    }

    printf("Thread[%d], cont: %d: \n", threadIndex, threadIndex == 0? c1 : c2);
}

void AtomicOps_ThreadFunc3(void *args){
    int i, transactionStatus, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());
    char * message = NULL;

    for (i = 0; i < NUM_VALUES; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus  == _XBEGIN_STARTED) {
            __sync_fetch_and_add(&globalCount, 1);
            _xend();
        } else { //... non-transactional fallback path...
            AtomicOps_IncAbort();
//            message = TSX_GetTransactionAbortMessage(transactionStatus);
//            printf("%s ... Was explicit abort? %d "
//                           "Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//            free(message);
            //i--;
        }
    }
}

void AtomicOps_ThreadFunc4(void *args){
    int i, transactionStatus, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());
    int auxVar;
    char * message = NULL;

    if (threadIndex == 0){
        while(sharedVariable < NUM_VALUES -1){
            for(i = 0; i < 1000; i++);
        }
        return;
    }

    for (i = 0; i < NUM_VALUES; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus == _XBEGIN_STARTED) {
            __sync_fetch_and_add(&sharedVariable, 1);
            _xend();
        } else { //... non-transactional fallback path...
            AtomicOps_IncAbort();
//            message = TSX_GetTransactionAbortMessage(transactionStatus);
//            printf("%s ... Was explicit abort? %d "
//                           "Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//            free(message);
            //i--;
            __sync_fetch_and_add(&sharedVariable, 1);
        }
    }
}

void AtomicOps_ThreadFunc5(void *args) {
    int i, transactionStatus, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self()), localCount = 0;
    int auxVar;
    char *message = NULL;

    printf("Thread[%d] reached barrier\n\n", threadIndex);
    pthread_barrier_wait(&barrier1);


    // hyper-thread
    start1:
    if (threadIndex == 1) {
        localCount = Transactions_CalculateValue(threadIndex);
        __sync_lock_test_and_set(&sharedVariable, localCount);
        //Finished computing its part
        hyperThreadFinished = 1;
        printf("Thread1 finished computing... waiting for main thread\n");
        pthread_barrier_wait(&barrier1);
        return;
    }

    // main thread
    start2:
    //for(i = 0; i < 1000; i++);
    printf("Thread0 starting transaction\n");
    transactionStatus = _xbegin();
    if (transactionStatus == _XBEGIN_STARTED) {
        localCount = Transactions_CalculateValue(threadIndex);

        // waiting for other thread to compute its part, but if not finished will abort
        while (!hyperThreadFinished){
            for(i = 0; i < 1000; i++);
        }

        if(localCount != sharedVariable){
//            _xabort(0xff);
        }
        _xend();

    } else { //... non-transactional fallback path...
        AtomicOps_IncAbort();
        if(!TSX_WasExplicitAbort(transactionStatus)){
            //its probably because the thread 1 has not yet finished, so we wait for him
            if(!hyperThreadFinished) {
                printf("Thread[%d] will wait because it failed...\n", threadIndex);
                pthread_barrier_wait(&barrier1);
            }
        }else{
            // manage soft error
        }
//        message = TSX_GetTransactionAbortMessage(transactionStatus);
//        printf("%s ... Was explicit abort? %d Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//        free(message);
        goto start2;
    }

    if(hyperThreadFinished) {
        printf("Thread[%d] will wait because it finished...\n", threadIndex);
        pthread_barrier_wait(&barrier1);
    }
    printf("Main thread finished\n");
}

void AtomicOps_ThreadFunc6(void *args){
    int i, transactionStatus, threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());
    int auxVar;
    char * message = NULL;

    if(threadIndex == 0) {
        printf("Address of myVar1 %ld \n", (long) &myVar1);
        printf("Address of myVar2 %ld \n", (long) &myVar2);
        printf("Address of e0 %ld \n", (long) &e0);
        printf("Address of e1 %ld \n", (long) &e1);
    }

    e0.v1 = e1.v1 = 0;

    for (i = 0; i < NUM_VALUES; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus  == _XBEGIN_STARTED) {
            if(threadIndex == 0){
                //myVar1++;
                e0.v1++;
            }else{
                //myVar2++;
                e1.v1++;
            }
            //e0.v1++;
            _xend();
        } else { //... non-transactional fallback path...
            AtomicOps_IncAbort();
//            message = TSX_GetTransactionAbortMessage(transactionStatus);
//            printf("%s ... Was explicit abort? %d "
//                           "Is retry possible? %d\n",
//                   message, TSX_WasExplicitAbort(transactionStatus), TSX_IsRetryPossible(transactionStatus));
//            free(message);
            //i--;
        }
    }
}

void AtomicOps_Test(){
    int test = 5;

    int numThreads = 2;
    void * threadFunc =
            test == 1 ? &AtomicOps_ThreadFunc1 :
            test == 2 ? &AtomicOps_ThreadFunc2 :
            test == 3 ? &AtomicOps_ThreadFunc3 :
            test == 4 ? &AtomicOps_ThreadFunc4 :
            test == 5 ? &AtomicOps_ThreadFunc5 :
            &AtomicOps_ThreadFunc6;

    pthread_barrier_init(&barrier1, NULL, 2);
    THREAD_UTILS_SetNumThreads(numThreads);
    THREAD_UTILS_CreateThreads();
    THREAD_UTILS_StartThreads(threadFunc, NULL);


    printf("Final Result: %ld , NUM_VALUES %d,  num of aborts: %ld, %% aborts: %f \n",
           globalCount, NUM_VALUES, numAborts, (double) (numAborts * 100) / NUM_VALUES);


    THREAD_UTILS_DestroyThreads();
}
