//
// Created by dperez on 22/05/17.
//

#include "BenchmarkSupport.h"

void BENCHMARK_SUPPORT_EvaluateTransactions(int numExecutions, int numThreads, int usingTM){
    int i = 0, successful = 0, successCount = 0;
    gdouble * times = g_malloc(sizeof(gdouble) * numExecutions);
    gdouble mean = 0, sd = 0;

    for(i =0; i < numExecutions; i++) {
        GTimer *timer = g_timer_new();
        successful = Transactions_Test(numThreads, usingTM) == 1;
        successCount += successful;

        g_timer_stop(timer);
        gulong fractional_part = 0;
        gdouble seconds_elapsed = g_timer_elapsed(timer, &fractional_part);
        gdouble milliseconds_elapsed = seconds_elapsed * 1000;
        mean += milliseconds_elapsed;
        times[i] = milliseconds_elapsed;
        g_timer_destroy(timer);

        printf("Milliseconds elapsed: %f "
                       "Errors detected: %d   "
                       "Was it Successful? %d \n", milliseconds_elapsed, errorsInjected, successful);
    }
    mean /= numExecutions;
    for(i = 0; i < numExecutions; i++){
        sd += abs(mean - times[i]);
    }
    sd /= numExecutions;

    printf("Mean Execution time: %f --- "
                   "Standard Deviation: %f --- "
                   "Successful executions: %d --- "
                   "Success Rate: %f \n", mean, sd, successCount, ((float) (successCount)) / numExecutions);

}
