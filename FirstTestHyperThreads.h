//
// Created by diego on 31/08/17.
//

#ifndef TSXPROJECT_FIRSTTESTHYPERTHREADS_H
#define TSXPROJECT_FIRSTTESTHYPERTHREADS_H

#endif //TSXPROJECT_FIRSTTESTHYPERTHREADS_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

void HyperThreads_PingPongTest(int useHyperThread);
void HyperThreads_QueueTest(ExecMode execMode);
void HyperThreads_TestAllCombinations();