
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


//#include "TestCoreAffinity.h"
#include "Test_HyperThread.h"

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

int main(int argc, char **argv){
    srand(time(NULL));
    int numExecutions =  atoi(argv[1]);

//    int value, v1, v2, v3, v4, v5;
//
//    v1 = 3; v2 = 4; v3 = 2; v4 = 5; v5 = 6;
//    value = v1 ^ v2 ^ v3 ^ v4 ^ v5;
//    printf("Binary: %s\n", byte_to_binary(value));
//
//    v1 = 3; v2 = 5; v3 = 2; v4 = 5; v5 = 7;
//    value = v1 ^ v2 ^ v3 ^ v4 ^ v5;
//    printf("Binary: %s\n", byte_to_binary(value));

    //HyperThreads_PingPongTest(0);

    // Heuristics
//    HyperThreads_QueueTest(notReplicated);
//    HyperThreads_QueueTest(replicatedSameThread);
//    HyperThreads_QueueTest(replicatedThreadsOptimally);

    // Every Time Simple Queue
//    HyperThreads_SetCheckFrequency(everyTime);
//    HyperThreads_SetQueueType(simpleQueueType);
//
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);

    // Every Time Section Queue
//    HyperThreads_SetCheckFrequency(everyTime);
//    HyperThreads_SetQueueType(sectionQueueType);
//
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);

    // Every Time Lynx Queue
    HyperThreads_SetCheckFrequency(everyTime);
    HyperThreads_SetQueueType(lynxqQueueType);

    HyperThreads_QueueTest(replicatedThreads);
    HyperThreads_QueueTest(replicatedHT);

//    HyperThreads_SetCheckFrequency(eachModuloTimes_Encoding);
//    HyperThreads_UseSectionQueueType(true);
//
//    HyperThreads_QueueTest(replicatedThreads);
//    HyperThreads_QueueTest(replicatedHT);

    return 0;
}

