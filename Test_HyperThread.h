//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

void HyperThreads_PingPongTest(int useHyperThread);
void HyperThreads_QueueTest(ExecMode execMode);

#endif //TSXPROJECT_TEST_HYPERTHREAD_H
