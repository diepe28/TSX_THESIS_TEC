
///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0


//#include "TestCoreAffinity.h"
#include "VectorMatrixMult.h"

//const char *byte_to_binary(long x)
//{
//    static char b[30];
//    b[0] = '\0';
//
//    int z;
//    for (z = 1<<25; z > 0; z >>= 1)
//    {
//        strcat(b, ((x & z) == z) ? "1" : "0");
//    }
//
//    return b;
//}



int main(int argc, char **argv){
    srand(time(NULL));
    int useHyperThread = atoi(argv[1]) != 0;

    int value, v1, v2, v3, v4, v5;
    long l1, l2, l3, l4, l5, result;
    float floatVar = 0.1f;
    int fl = *(int*)&floatVar;

    v1 = 3; v2 = 4; v3 = 2; v4 = 5; v5 = 6;
//    value = v1 ^ v2 ^ v3 ^ v4 ^ v5;
//    printf("Binary: %s\n", byte_to_binary(value));
//
//    v1 = 3; v2 = 5; v3 = 2; v4 = 5; v5 = 6;
//    value = v1 ^ v2 ^ v3 ^ v4 ^ v5;
//    printf("Binary: %s\n", byte_to_binary(value));


//    l1 = 2; v1 = 4;
//    printf("Binary: %s\n", byte_to_binary(l1));
//    printf("Binary: %s\n", byte_to_binary(v1));

    //result = l1 ^ v1;
    //printf("Binary: %s\n", byte_to_binary(result));
    //HyperThreads_TestAllCombinations();

    //printf("Binary: %s\n", byte_to_binary(fl));
    Vector_Matrix_Mult(useHyperThread);

    return 0;
}

