
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
    int producerCoreHT, consumerCoreHT, producerCore, consumerCore;

    if(argc == 5){
        printf("There are parameters\n");
        producerCore = atoi(argv[1]);
        consumerCore = atoi(argv[2]);
        producerCoreHT = atoi(argv[3]);
        consumerCoreHT = atoi(argv[4]);
    }else{ // my machine
        producerCore = 0;
        consumerCore = 1;
        producerCoreHT = 0;
        consumerCoreHT = 2;
    }

    Vector_Matrix_Mult(producerCore, consumerCore, producerCoreHT, consumerCoreHT);
    return 0;
}

/*
  Try to see how much do we gain by not writing back ALREADY CONSUMED on consumer, the producer has to change a bit.
  Then try to see how to accomplish that, using just the pointers.
  Try to apply the technique to HPCCG.
 * */

/*
 How can we bring TSX to the table again? maybe just for the non-volatile stores, since most of the times the consumer
 just reads the values. He can read it first, start a transaction, calculate its own value and the compare... something
 like that.
 * */