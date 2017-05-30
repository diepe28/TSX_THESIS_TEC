#ifndef TEST_TSX_H
#define TEST_TSX_H

#include "UtilFunctionsTSX.h"
#include "ThreadUtils.h"

#define NUM_VALUES 999

extern long globalCount;
extern int const FOR_VALUE;
extern int values[NUM_VALUES];
extern double * results;
extern double const ERROR_PROBABILITY;
extern int injectedErrors;

void* Atomicity_incrementCounterWithTM(void *arg);

void* Atomicity_incrementCounter(void *arg);

int Atomicity_Test(int nTimes,int usingTM) ;

void Transactions_InitValues();

void Transactions_DestroyValues();

double Transactions_CalculateValue(int i);

void* Transactions_FuncWithTM(void *args);

void* Transactions_Func(void *args);

void Transactions_Test(int numThreads, int usingTM);

#endif //TEST_TSX_H
