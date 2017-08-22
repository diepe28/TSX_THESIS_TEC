//
// Created by diego on 27/07/17.
//

#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include "lynxq.h"

#define LYNXQ_QUEUE_SIZE (1<<21) /* 2MB */

#define SECTION_SIZE 1000
#define NUM_SECTIONS 4

extern long producerCount;
extern long consumerCount;

typedef enum{
    ExeMode_notReplicated,
    ExeMode_replicatedSameThread,
    ExeMode_replicatedThreadsOptimally,
    ExeMode_replicatedThreads,
    ExeMode_replicatedHyperThreads,
}ExecMode;

typedef struct{
    long content[SECTION_SIZE];
    bool isReadMode;
}Section;

/////////////// Multi section Queue

typedef struct{
    volatile  int enqPtr;
    volatile int deqPtr;
    int enqueueSection;
    int dequeueSection;
    volatile Section sections[NUM_SECTIONS];
}SectionQueue;

SectionQueue SectionQueue_Init();
void SectionQueue_Enqueue(SectionQueue* this, long value);
long SectionQueue_Dequeue(SectionQueue* this);

void SectionQueue_WastedInst();

/////////////// Simple Queue
#define SIMPLE_QUEUE_MAX_ELEMENTS 10000

typedef struct{
    long content[SIMPLE_QUEUE_MAX_ELEMENTS];
    volatile int enqPtr;
    volatile int deqPtr;
}SimpleQueue;

SimpleQueue SimpleQueue_Init();
void SimpleQueue_Enqueue(SimpleQueue* this, long value);
long SimpleQueue_Dequeue(SimpleQueue* this);

#endif //TSXPROJECT_SIMPLEQUEUE_H
