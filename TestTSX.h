#ifndef TEST_TSX_H
#define TEST_TSX_H

#include "UtilFunctionsTSX.h"
#include "ThreadUtils.h"

#define NUM_VALUES 1000000

extern long globalCount;
extern int values[NUM_VALUES];
extern int errors[NUM_VALUES];
extern int errorsInjected;
extern double * results;
extern int const ERROR_PROBABILITY;

void* Atomicity_incrementCounterWithTM(void *arg);

void* Atomicity_incrementCounter(void *arg);

int Atomicity_Test(int nTimes,int usingTM) ;

void Transactions_InitValues();

void Transactions_DestroyValues();

double Transactions_CalculateValue(int i);

void* Transactions_FuncWithTM(void *args);

void* Transactions_Func(void *args);

int Transactions_Test(int numThreads, int usingTM);

#endif //TEST_TSX_H
