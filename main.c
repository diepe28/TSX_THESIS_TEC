#include "BenchmarkSupport.h"

int errorsInjected = 0;

int calcValue() {
    int r = rand() % 100, result = 1;
    //r = r < ERROR_PROBABILITY? rand() % 100 : r;

    if (r < ERROR_PROBABILITY) {
        errorsInjected++;
        result = 2;
    }

    return result;
}

void TestFunction(){
    int i, v1 = 5, v2 = 5, status, n = 100, result = 0, errorsDetected;
    int n_tries, max_tries = 5;

    for(i =0; i < n; i++){
        errorsDetected = 0;

        printf("Executing iteration %d result %d \n", i, result);
        for (n_tries = 0; n_tries < max_tries; n_tries++)
        {
            status = _xbegin();
            if (status == _XBEGIN_STARTED || !(status & _XABORT_RETRY))
                break;
        }
        if (status == _XBEGIN_STARTED){
            v1 = calcValue();
            v2 = calcValue();

            result += v1;
            if(v1 != v2){
                _xabort(0xff);
            }

            _xend();
        }else{//fallback path
            if((status & _XABORT_EXPLICIT) && (_XABORT_CODE(status) == 0xff)){
                // everything is reverted to the previous state, even the rand() function will be the same
                // printf("Transaction aborted! value of result %d \n", result);
                i--; // try again
                // this is necessary because otherwise in the next iteration the result
                // will be the same as the one that failed
                calcValue(); calcValue();
            }else{
                // non transactional execution
                result += calcValue();
            }
        }
    }

    printf("Result should be %d, it is %d, errors injected %d\n", n, result, errorsInjected);

}

///
/// Main method of the prototype
/// Remember in order to get the documentation run
/// > doxygen configfile
/// It calls Atomicity_Test to test access to a critical section using Transactional Memory and
/// Transactions_Test to test how to recover from errors using Transactional Memory.
/// \return 0
int main(){
    srand(time(NULL));

    //Atomicity_Test(10, 0);
    Transactions_Test(1,1);
    //BENCHMARK_SUPPORT_EvaluateTransactions();

    //TestFunction();

    return 0;
}

