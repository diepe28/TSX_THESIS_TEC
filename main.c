#include "TestTSX.h"
#include <stdio.h>

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// \return

double dRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}
int main(){
    srand(time(NULL));

    //Atomicity_Test(10, 0);
    Transactions_Test(5,1);

//    int i;
//    int r = rand() % 100;
//    for (i = 0; i < 1000; i++){
//        r = rand() % 100;
//        if(r < 2)
//            printf("%d ", r);
//    }

    return 0;
}

