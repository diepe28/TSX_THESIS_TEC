//
// Created by diego on 27/07/17.
//

#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include "lynxq.h"
#include "rtm.h"

//#define LYNXQ_QUEUE_SIZE (1<<21) /* 2MB */
#define LYNXQ_QUEUE_SIZE (1<<20) /* 2MB */

#define SECTION_SIZE 1000
#define NUM_SECTIONS 4

#define SIMPLE_QUEUE_MAX_ELEMENTS 4096
//#define SIMPLE_SYNC_QUEUE_SIZE 10000
#define SIMPLE_SYNC_QUEUE_SIZE 10
#define SIMPLE_SYNC_QUEUE_LAST_VALUE 9
#define NUM_RUNS 5
#define MODULO 5

void SetThreadAffinity(int threadId);

typedef enum {
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

extern long volatile producerCount;
extern long volatile consumerCount;

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
    long* content;
    double padding0[15];
    volatile int enqPtr;
    double padding1[15];
    volatile int deqPtr;
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

void Global_SetQueueType(QueueType newQueueType);
void Global_SetCheckFrequency(CheckFrequency newCheckFrequency);

#endif //TSXPROJECT_SIMPLEQUEUE_H
