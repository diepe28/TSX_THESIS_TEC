
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0

// -D CMAKE_C_COMPILER=/usr/bin/clang-4.0 -D CMAKE_CXX_COMPILER=/usr/bin/clang++-4.0
//-D CMAKE_C_COMPILER=/usr/bin/gcc -D CMAKE_CXX_COMPILER=/usr/bin/g++

//#include "TestCoreAffinity.h"
#include "VectorMatrixMult.h"

int main(int argc, char **argv){
    srand(time(NULL));
    int producerCore, consumerCore;

    if(argc == 3){
        printf("There are parameters\n");
        producerCore = atoi(argv[1]);
        consumerCore = atoi(argv[2]);
    }else{
        // using hyper-threads
        producerCore = 1;
        consumerCore = 3;
    }

    Vector_Matrix_Mult(producerCore, consumerCore);
    return 0;
}

/*
 * Main memory is already protected. []

Hyper-threads vs different cores, not against threads, do not mess witbuenoh the concept of already known thread.

The objectives are what should go in the beggining. Not what I wrote as the main contributions...

Causes and consequences,  Jean Claude Laprie (what is an error, what is a failure, definition). Slides, an error might result into a failure, a wrong behaviour, to not be able to deliver the correct result. Fault leads to an error and probably to a failure.

Thread-local or in-thread, thread-local sounds better, be consistent.

Sofware replication, I will still have a small paragraph about ILR to define what it is.

More general comment, at the beggining of each chapter, is good to have a short paragraph what the chapter is going to be. A summarize of the paragraph. Like an abstract, like the outline of the chapter.

They ressemble DAFT, the figures. Mention that.

Correcting soft errors, it looks like is not linked. Just say either Triple, more concise about the checkpointing, dont mention error detection again, I would not say since transient fault the cost of recuperating is not tipically the problem, checkpointing the fact of keeping the checkpoint is costly, you have to save... maybe exactly not the recovery but to keep the checkpoint.


Hyper-thread less details about the number of size of the chip.

Simultaneous multi-threading, a general technique and then go to specificly hyper-thread of intel. Mention the first time that is intels, and then just say Hyper-threading

Shoestring, mention ILR do not say another concept.

MPI ranks link to processes, not necesesarly to nodes.

Haft is mentioned in the previous paragraph, like explained two times. Probably merge the paragraphs.

Main comment of RMT, the fact of. Explicitly say what was already defined in the previous chapter. The type of communication. Also is not uniderectional.

Mention we are going to be like that technique.

There is no need for artificial handlers, there might be an external process that handles it. That

The first mention the user of hyper-threads with RMT, but finish with a conclusions, it can it to the next section.


The problem definion might change,

Not authors in general, several authros.

Programming models instead of schemes.

Not say instructions, communication instructions, function call... try to find a better, not just an instruction like in ILR.

Say that an iterative process might not reach the good result.

When we mention again not using ILR in the leading process, why not, why is it bad.

The solution was not designed for hyper-threads, it was a general solution to specifically to hyper-threads. Another point in favor.

In the hyphotesis, mention the how many cicles it takes to access of level 1 or level 3 cache, probably papers about it, and then make the hyphotesis, like based on that latency we can expect an improvement of how much.


We dont care, at least for now, of fault injection.

For test methododoly, hpc applications representative, different machines like the ones in Grid 5000, hardware counters.

 If for example the configuration of hyper-threads is not as good as another configuration with different cores... well if is not as good still that different core
 can be used for another purpose, there are cases where hyper-threads do not help but having a new core will definitely help so if is the same or a bit worse then we
use that other core for another purpose.

 One idea is to let the producer produce without sync, since there are only a few times (at least with a 1024 queue size) the producer actually has to wait for the
 consumer. He could write to another place, another queue maybe, when the consumer reads a wrong value if is ALREADY_CONSUMED it waits for the producer but before
 deciding is a soft error he writes the first value of this other queue, if it is the same
 * */