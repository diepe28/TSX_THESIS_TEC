
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

void printMatrix(int rows, int cols, long matrix[rows][cols]){
    int i,j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            printf("%ld ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
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

    //HyperThreads_TestAllCombinations();

    Vector_Matrix_Mult();

    return 0;
}

