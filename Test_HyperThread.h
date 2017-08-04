//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

typedef enum {
    everyTime,
    eachModuloTimes,
    eachModuloTimes_Encoding
}CheckFrequency;

typedef enum {
    simpleQueueType,
    sectionQueueType,
    lynxqQueueType
}QueueType;

void HyperThreads_PingPongTest(int useHyperThread);
void HyperThreads_QueueTest(ExecMode execMode);
void HyperThreads_SetQueueType(QueueType);
void HyperThreads_SetCheckFrequency(CheckFrequency);

#endif //TSXPROJECT_TEST_HYPERTHREAD_H
