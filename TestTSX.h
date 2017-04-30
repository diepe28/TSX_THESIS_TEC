#ifndef TEST_TSX_H
#define TEST_TSX_H

#include "UtilFunctionsTSX.h"
#include "ThreadUtils.h"

#define NUM_VALUES 9999

extern long globalCount;
extern int const FOR_VALUE;
extern int values[NUM_VALUES];
extern double * results;

void* Atomicity_incrementCounterWithTM(void *arg);

void* Atomicity_incrementCounter(void *arg);

int Atomicity_Test(int nTimes,int usingTM) ;

int Transactions_InitValues();

double Transactions_CalculateValue(int i);

void* Transactions_ThreadFunc(void *args);

void Transactions_Test(int numThreads);

#endif //TEST_TSX_H
