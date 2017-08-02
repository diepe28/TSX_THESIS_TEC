//
// Created by diego on 02/08/17.
//

#ifndef TSXPROJECT_TEST_QUEUE_H
#define TSXPROJECT_TEST_QUEUE_H

#include "ThreadUtils.h"
#include "TestTSX.h"
#include <glib.h>
#include "SimpleQueue.h"

void TestQueue_ThreadProducer(void * arg);
void TestQueue_SimpleTest(int useHyperThread);

#endif //TSXPROJECT_TEST_QUEUE_H
