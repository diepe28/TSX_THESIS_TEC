//
// Created by diego on 19/07/17.
//

#ifndef TSXPROJECT_TESTATOMICOPS_H
#define TSXPROJECT_TESTATOMICOPS_H

//
// Created by diego on 19/07/17.
//

#include "TestAtomicOps.h"
#include "TestTSX.h"

////////////////////////// TEST atomic operation with TSX

void AtomicOps_IncAbort();

void AtomicOps_ThreadFunc1(void *args);
void AtomicOps_ThreadFunc2(void *args);

void AtomicOps_ThreadFunc3(void *args);
void AtomicOps_ThreadFunc4(void *args);

void AtomicOps_ThreadFunc5(void *args);
void AtomicOps_ThreadFunc6(void *args);

void AtomicOps_Test();


#endif //TSXPROJECT_TESTATOMICOPS_H
