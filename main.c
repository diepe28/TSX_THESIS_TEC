#include "TestTSX.h"
#include <stdio.h>

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0
int main(){
    srand(time(NULL));

    Atomicity_Test(10, 0);
    Transactions_Test(5,1);

    return 0;
}

