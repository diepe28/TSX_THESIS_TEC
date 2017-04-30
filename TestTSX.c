#include "TestTSX.h"
#include<stdio.h>

//http://pmarlier.free.fr/gcc-tm-tut.html
//http://www-users.cs.umn.edu/~boutcher/stm/
//compile with '-fgnu-tm'
//gcc -o exe main.c -fgnu-tm

long globalCount = 0;
int const FOR_VALUE = 9999;
int values[NUM_VALUES];
double * results;

void* Atomicity_incrementCounterWithTM(void *arg) {
    unsigned long i;
    int status;

    for(i = 0; i<FOR_VALUE;i++)
    {
        status = _xbegin ();
        if (status == _XBEGIN_STARTED) {
            globalCount++;
            _xend();
        }
        else
        {
            //... non-transactional fallback path...
            //globalCount++;
            //printf("Transaction creation failed: %s\n", getTransactionAbortMessage(status));
            i--;
            continue; // retry
        }
    }

    return NULL;
}

void* Atomicity_incrementCounter(void *arg) {
    unsigned long i;

    for(i = 0; i<FOR_VALUE;i++)
    {
        globalCount++;
    }

    return NULL;
}

int Atomicity_Test(int nTimes,int usingTM) {
    int i = 0, passedTests = 0;
    long finalResult;

    THREAD_UTILS_SetNumThreads(15);
    THREAD_UTILS_CreateThreads();

    finalResult = THREAD_UTILS_GetNumThreads() * FOR_VALUE;

    for (i = 0 ;i < nTimes; i++) {
        globalCount = 0;
        THREAD_UTILS_StartThreads(usingTM ? &Atomicity_incrementCounterWithTM : &Atomicity_incrementCounter, NULL);
        if (finalResult == globalCount) {
            passedTests++;
        }else {
            printf("Result should be %ld, but it is: %ld \n", finalResult, globalCount);
        }
    }

    THREAD_UTILS_DestroyThreads();
    printf("%d out of %d executions successfull\n",passedTests, nTimes);
    return passedTests;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

int Transactions_InitValues(){
    results = (malloc(sizeof(double) * THREAD_UTILS_GetNumThreads()));

    int i = 0;
    for (; i < NUM_VALUES; i++) {
        values[i] = i;
    }
    for(i = 0; i < THREAD_UTILS_GetNumThreads(); i++)
        results[i] = 0;
}

double Transactions_CalculateValue(int n) {
    //return i * i;
    int i = 0;
    for(i = 0; i < n; i++){
        i += n;
        i -=n;
    }
    return 1;
}

///
/// The function each tread executes
///
void* Transactions_ThreadFunc(void *args) {
    int i, startIndex = 0, endIndex = 0, transactionStatus, aux;
    double result = 0, temp, temp2;
    int threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());

    startIndex = THREAD_UTILS_GetRangeFromThreadId(threadIndex, &endIndex, NUM_VALUES);
    for (i = startIndex; i < endIndex; i++) {
        transactionStatus = _xbegin();
        if (transactionStatus == _XBEGIN_STARTED) {
            aux = transactionStatus;
            temp = Transactions_CalculateValue(values[i]);
            temp2 = Transactions_CalculateValue(values[i]);
            if (temp != temp2) {
                _xabort(9);
            }
            result += temp;
            _xend();
        } else {
            //... non-transactional fallback path...
            //printf("Transaction creation failed: %s\n", getTransactionAbortMessage(transactionStatus));
            i--;
            continue; // retry
        }
    }
    results[threadIndex] = result;
}

///
/// A function that test how transactions work in TSX
///
void Transactions_Test(int numThreads){
    int i;
    double finalResult = 0;

    THREAD_UTILS_SetNumThreads(numThreads);
    Transactions_InitValues();

    THREAD_UTILS_CreateThreads();
    THREAD_UTILS_StartThreads(&Transactions_ThreadFunc, NULL);

    for(i =0; i < numThreads; i++){
        finalResult += results[i];
    }

    THREAD_UTILS_DestroyThreads();
    free(results);
    printf("The final result is: %f\n", finalResult);
}
