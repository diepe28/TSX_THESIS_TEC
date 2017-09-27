//
// Created by diego on 27/07/17.
//

#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include "lynxq.h"
#include "rtm.h"
#include "pcqueue.h"

#define SIMPLE_QUEUE_MAX_ELEMENTS 2048

#define SECTION_SIZE 1000
#define NUM_SECTIONS 4

#define LYNXQ_QUEUE_SIZE (1<<21) /* 2MB */

#define SIMPLE_SYNC_QUEUE_SIZE 512
#define ALREADY_CONSUMED -2

#define NUM_RUNS 5
#define MODULO 5

void SetThreadAffinity(int threadId);

typedef enum {
    CheckFrequency_SynchronousOneVar,
    CheckFrequency_SynchronousTwoVars,
    CheckFrequency_SynchronousQueue,
    CheckFrequency_Volatile,
    CheckFrequency_VolatileEncoding,

    CheckFrequency_everyTime,
    CheckFrequency_eachModuloTimes,
    CheckFrequency_Encoding
}CheckFrequency;

typedef enum {
    QueueType_Simple,
    QueueType_SimpleSync,
    QueueType_Section,
    QueueType_lynxq
}QueueType;

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

/////////////// Multi section Queue ///////////////

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

/////////////// Simple Queue ///////////////

typedef struct{
    long content[SIMPLE_QUEUE_MAX_ELEMENTS];
    double padding0[15];
    volatile int enqPtr;
    double padding1[15];
    volatile int deqPtr;
}SimpleQueue;

SimpleQueue SimpleQueue_Init();
void SimpleQueue_Enqueue(SimpleQueue* this, long value);
long SimpleQueue_Dequeue(SimpleQueue* this);

/////////////// Simple Without Sync Queue ///////////////
typedef struct{
    //long content[SIMPLE_QUEUE_MAX_ELEMENTS];
    int deqPtr;
    double padding0[15];
    int enqPtr;
    double padding1[15];
    volatile long* content;
    double padding2[15];
    volatile int checkState; // 0 not verified, 1 no soft error, 1 soft error
    double padding3[15];
    volatile long currentValue;
}SimpleSyncQueue;

SimpleSyncQueue SimpleSyncQueue_Init();
void SimpleSyncQueue_Enqueue(SimpleSyncQueue* this, long value);
long SimpleSyncQueue_Dequeue(SimpleSyncQueue* this);
SimpleSyncQueue SimpleSyncQueue_Destroy(SimpleSyncQueue* this);

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// GLOBAL FUNCS, VARS FOR QUEUES //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
SectionQueue sectionQueue;
SimpleQueue simpleQueue;
SimpleSyncQueue simpleSyncQueue;
lynxQ_t lynxQ1;

CheckFrequency checkFrequency;
QueueType queueType;

volatile long producerCount;
volatile long consumerCount;

void Global_SetQueueType(QueueType newQueueType);
void Global_SetCheckFrequency(CheckFrequency newCheckFrequency);

#endif //TSXPROJECT_SIMPLEQUEUE_H
