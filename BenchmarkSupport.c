//
// Created by dperez on 22/05/17.
//

#include "BenchmarkSupport.h"

void BENCHMARK_SUPPORT_EvaluateTransactions(int numExecutions, int numThreads, int usingTM){
    int i = 0, successful = 0;
    gdouble mean = 0;

    for(i =0; i < numExecutions; i++) {
        GTimer *timer = g_timer_new();
        successful = Transactions_Test(numThreads, usingTM) == 1;
        g_timer_stop(timer);
        gulong fractional_part = 0;
        gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
        gdouble milliseconds_elapsed = seconds_elapsed * 1000;
        mean += milliseconds_elapsed;
        g_timer_destroy(timer);
        printf("Was it successful? %d ... milliseconds_elapsed elapsed: %f\n", successful, milliseconds_elapsed);
    }

    mean /= numExecutions;

    printf("Mean Execution time: %f\n", mean);

}
