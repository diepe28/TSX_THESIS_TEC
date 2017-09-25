
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


//#include "TestCoreAffinity.h"
#include "VectorMatrixMult.h"

int main(int argc, char **argv){
    srand(time(NULL));
    int useHyperThread = 1; //atoi(argv[1]) != 0;

    Vector_Matrix_Mult(useHyperThread);
    return 0;
}

