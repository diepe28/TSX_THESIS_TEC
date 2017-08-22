//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

#define MODULO 5

typedef enum {
    CheckFrequency_everyTime,
    CheckFrequency_eachModuloTimes,
    CheckFrequency_Encoding
}CheckFrequency;

typedef enum {
    QueueType_Simple,
    QueueType_Section,
    QueueType_lynxq
}QueueType;

void HyperThreads_PingPongTest(int useHyperThread);
void HyperThreads_QueueTest(ExecMode execMode);
void HyperThreads_SetQueueType(QueueType);
void HyperThreads_SetCheckFrequency(CheckFrequency);
void HyperThreads_TestAllCombinations();
void Vector_Matrix_Mult();
#endif //TSXPROJECT_TEST_HYPERTHREAD_H
