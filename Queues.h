//
// Created by diego on 27/07/17.
//


#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H

#define __USE_GNU 1
#define _GNU_SOURCE

#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include "rtm.h"

#define SIMPLE_QUEUE_MAX_ELEMENTS 2048



#define SECTION_SIZE 1000
#define NUM_SECTIONS 4

//#define SIMPLE_SYNC_QUEUE_SIZE 256
//#define SIMPLE_SYNC_QUEUE_SIZE 512
#define SIMPLE_SYNC_QUEUE_SIZE 1024
//#define SIMPLE_SYNC_QUEUE_SIZE 2048
//#define SIMPLE_SYNC_QUEUE_SIZE 4096
//#define SIMPLE_SYNC_QUEUE_SIZE 8192
//#define SIMPLE_SYNC_QUEUE_SIZE 16384

#define ALREADY_CONSUMED -1
#define NUM_RUNS 5
#define MODULO 5

void SetThreadAffinity(int threadId);

typedef enum {
    CheckFrequency_SynchronousOneVar,
    CheckFrequency_SynchronousTwoVars,
    CheckFrequency_SynchronousQueue,
    CheckFrequency_RHT_NoVolatiles,
    CheckFrequency_Volatile,
    CheckFrequency_RHT_Volatiles,
    CheckFrequency_RHT_VolatilesImproved,
    CheckFrequency_VolatileNoSyncNoModulo,
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
    ExeMode_replicatedThreads
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
    volatile int deqPtr;
    double padding0[15];
    volatile int enqPtr;
    double padding1[15];
    volatile long* content;
    double padding2[15];
    volatile int checkState;
    double padding3[15];
    volatile long volatileValue;
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

CheckFrequency checkFrequency;
QueueType queueType;

volatile long producerCount;
volatile long consumerCount;

void Global_SetQueueType(QueueType newQueueType);
void Global_SetCheckFrequency(CheckFrequency newCheckFrequency);

#endif //TSXPROJECT_SIMPLEQUEUE_H
