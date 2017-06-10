#include "BenchmarkSupport.h"

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0
int main(int argc, char **argv){
    srand(time(NULL));

    int numExecutions =  atoi(argv[1]);
    int numThreads = 0;
    int usingTM = 0; // default 0
    int i = 0;

    printf("Num Executions %d\n", numExecutions);
executionPhase:
    numThreads = 1; // default 1
    i = 0;
    do{
        printf("\n//////////////////////// %d Thread - TXS? %d ////////////////\n", numThreads, usingTM);
        BENCHMARK_SUPPORT_EvaluateTransactions(numExecutions, numThreads, usingTM);
        i += 2;
        numThreads = i;
    }while(i < 5);

    if(!usingTM) {
        usingTM = 1;
        goto executionPhase;
    }

    //Transactions_Test(numThreads, usingTM);

    return 0;
}

