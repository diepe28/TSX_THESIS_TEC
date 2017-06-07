#include "BenchmarkSupport.h"

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0
int main(){
    srand(time(NULL));

    int numExecutions = 10;
    int numThreads = 4;
    int usingTM = 1;

    //BENCHMARK_SUPPORT_EvaluateTransactions(numExecutions, numThreads, usingTM);

    printf("Successful? %d \n", Transactions_Test(numThreads, usingTM));

    return 0;
}

