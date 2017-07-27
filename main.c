#include "BenchmarkSupport.h"
#include "TestCoreAffinity.h"
#include "Test_HyperThread.h"
#include "SimpleQueue.h"

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


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
    SimpleQueue q1 = SimpleQueue_Init();
    SimpleQueue_Enqueue(&q1, 3);
    SimpleQueue_Enqueue(&q1, 4);
    SimpleQueue_Enqueue(&q1, 5);

    SimpleQueue_Enqueue(&q1, 6);
    SimpleQueue_Enqueue(&q1, 7);
    SimpleQueue_Enqueue(&q1, 8);

    for(i = 0; i < 6; i++) {
        printf("Dequeue: %d\n", SimpleQueue_Dequeue(&q1));
    }


    return 0;
}

