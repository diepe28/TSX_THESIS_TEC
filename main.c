
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


//#include "TestCoreAffinity.h"
#include "Test_HyperThread.h"

int main(int argc, char **argv){
    srand(time(NULL));
    int numExecutions =  atoi(argv[1]);

    //HyperThreads_PingPongTest(0);

//    HyperThreads_QueueTest(notReplicated);
//    HyperThreads_QueueTest(replicatedSameThread);
//    HyperThreads_QueueTest(replicatedThreadsOptimally);

//    HyperThreads_CheckEveryTime(true);
//    HyperThreads_UseSectionQueueType(true);
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);
//
//    HyperThreads_CheckEveryTime(false);
//    HyperThreads_UseSectionQueueType(true);
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);
//
//    HyperThreads_CheckEveryTime(true);
//    HyperThreads_UseSectionQueueType(false);
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);

    HyperThreads_CheckEveryTime(false);
    HyperThreads_UseSectionQueueType(false);

    //HyperThreads_QueueTest(replicatedThreads);
    HyperThreads_QueueTest(replicatedHT);

    return 0;
}

