
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


#include "TestCoreAffinity.h"
#include "Test_HyperThread.h"
#include "Test_Queue.h"

int main(int argc, char **argv){
    //srand(time(NULL));
    int numExecutions =  atoi(argv[1]);
    int i;
    //BENCHMARK_SUPPORT_EvaluateTransactions(numExecutions);
    //Replication_Tests();
    //AtomicOps_Test();
    //CoreAffinity_View(2, 1);

    //CoreAffinity_Replication_Test(normal);
    //CoreAffinity_Replication_Test(normallyReplicated);
    //CoreAffinity_Replication_Test(normallyReplicatedWithHT);
    //CoreAffinity_Replication_Test(hyperReplicated);

    //HyperThreads_PingPongTest(0);
    HyperThreads_PingPongTest(1);
    //TestQueue_SimpleTest(0);
    //BENCHMARK_SUPPORT_EvaluateTransactions(1);
    //SimpleQueue q1 = SimpleQueue_Init();


    return 0;
}

