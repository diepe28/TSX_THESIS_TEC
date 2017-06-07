#include "TestTSX.h"

//http://pmarlier.free.fr/gcc-tm-tut.html
//http://www-users.cs.umn.edu/~boutcher/stm/
//compile with '-fgnu-tm'
//gcc -o exe main.c -fgnu-tm

long globalCount = 0;
int values[NUM_VALUES];
int errorsDetected;
pthread_mutex_t errorsMutex;
double * results;
const int ERROR_PROBABILITY = 1000000;

/// The function each threads executes when testing atomicity using transactional memory
/// \param arg the thread argument
/// \return NULL
void* Atomicity_incrementCounterWithTM(void *arg) {
    unsigned long i;
    int status;

    for(i = 0; i<NUM_VALUES;i++)
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

/// The function each threads executes when testing atomicity NOT using transactional memory
/// \param arg the thread argument
/// \return NULL
void* Atomicity_incrementCounter(void *arg) {
    unsigned long i;

    for(i = 0; i<NUM_VALUES;i++)
    {
        globalCount++;
    }

    return NULL;
}

/// Test atomicity using transactional memory intel extensions. Basically a global counter is incremented concurrently
/// multiple threads, using TM this always produces the same result and otherwise it is not a deterministic output.
/// \param nTimes the number of times the test executes
/// \param usingTM 0 mean don't use Transactional Memory and
/// \return the number of passed tests.
int Atomicity_Test(int nTimes,int usingTM) {
    int i = 0, passedTests = 0;
    long finalResult;

    THREAD_UTILS_SetNumThreads(15);
    THREAD_UTILS_CreateThreads();

    finalResult = THREAD_UTILS_GetNumThreads() * NUM_VALUES;

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

/// Initializes all values required for testing recovery from error using transactional memory
/// \return
void Transactions_InitValues(){
    results = (malloc(sizeof(double) * THREAD_UTILS_GetNumThreads()));
    errorsDetected = 0;

    int i = 0;
    for (; i < NUM_VALUES; i++) {
        values[i] = i;
    }
    for(i = 0; i < THREAD_UTILS_GetNumThreads(); i++)
        results[i] = 0;
}

/// Destroyes all values that were allocated in Transaction_InitValues
/// \return
void Transactions_DestroyValues(){
    free(results);
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/// Performs the calculus on each iteration, here is where a soft error might happen. We introduce a probability that
/// the result is not always the same (a soft error). For now it does not matter what it is calculated what matters is
/// that if error happens it has to be identified and recovered.
/// \param n a parameter that dictates the current calculus. This function should always return the same result for a
/// given n value, this is where with a probability we might choose to return something else indicating an error.
/// \return Most of times (using the given probability) the correct value for n value.
double Transactions_CalculateValue(int n) {
    int r = rand() % ERROR_PROBABILITY, i;
    double result = n;

    for(i = 2; i < 1000; i++){
        result *= i;
    }

    if (r < 1) { // probability of 1/ERROR_PROBABILITY
        result = 2;
    }else {
        result = 1;
    }

    return result;
}

///
/// The function each tread executes when testing detection and correction of soft errors using transactional memory
/// \param args arguments for the function
/// \return NULL
void* Thread_FuncWithTM(void *args) {
    int i, startIndex, endIndex, transactionStatus,v1, v2, retriedTimes = 0,
            threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self()) ;
    double result = 0;
    char * message = NULL;

    startIndex = THREAD_UTILS_GetRangeFromThreadId(threadIndex, &endIndex, NUM_VALUES);
    for (i = startIndex; i < endIndex; i++) {
        //printf("THREAD[%d]: Iteration %d, value %d\n", threadIndex, i - startIndex, (int) result);

        transactionStatus = _xbegin();
        if (transactionStatus  == _XBEGIN_STARTED) {
            v1 = (int) Transactions_CalculateValue(values[i]);
            v2 = (int) Transactions_CalculateValue(values[i]);

            //if (v1 != v2) {
            if (v1 == 2 || v2 == 2) {
                _xabort(0xff);
            }
            result += v1;
            retriedTimes = 0;
            _xend();
        } else { //... non-transactional fallback path...

            //message = TSX_GetTransactionAbortMessage(transactionStatus);
            //printf("Transaction creation failed: %s\n", message);
            //free(message);

            // this executes the rand() so the next iteration does not fail again.
            Transactions_CalculateValue(values[i]); Transactions_CalculateValue(values[i]);

            if(retriedTimes++ < 5 ){
                if(TSX_WasExplicitAbort(transactionStatus)) {
                    i--; // retry iteration
                    pthread_mutex_lock(&errorsMutex);
                    errorsDetected++;
                    pthread_mutex_unlock(&errorsMutex);
                    continue;
                }
                if(TSX_IsRetryPossible(transactionStatus)){
                    i--; // retry iteration
                    continue;
                }
            }

            // Non transactional execution
            v1 = Transactions_CalculateValue(values[i]);
            result += v1;
            if(v1 == 2) {
                printf("Executing without transactions an error occurred, retriedTimes %d \n", retriedTimes);
            }
        }

    }
    results[threadIndex] = result;
}

void* Thread_Func(void *args) {
    int i, startIndex = 0, endIndex = 0;
    double result = 0, v1;
    int threadIndex = THREAD_UTILS_GetThreadIndex(pthread_self());

    startIndex = THREAD_UTILS_GetRangeFromThreadId(threadIndex, &endIndex, NUM_VALUES);
    for (i = startIndex; i < endIndex; i++) {
        v1 = Transactions_CalculateValue(values[i]);
        result += v1;
    }

    results[threadIndex] = result;
}


/// Tests the transactional memory extensions of Intel for correction of soft errors.
/// \param numThreads the number of threads to be created.
int Transactions_Test(int numThreads, int usingTM){
    int i;
    double finalResult = 0;

    THREAD_UTILS_SetNumThreads(numThreads);
    Transactions_InitValues();

    THREAD_UTILS_CreateThreads();
    THREAD_UTILS_StartThreads(usingTM? &Thread_FuncWithTM : &Thread_Func, NULL);

    for(i =0; i < numThreads; i++){
        finalResult += results[i];
    }

    THREAD_UTILS_DestroyThreads();
    Transactions_DestroyValues();

    printf("The final result is: %f, errorsDetected: %d\n", finalResult, errorsDetected);
    return finalResult == NUM_VALUES;
}
