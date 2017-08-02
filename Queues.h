//
// Created by diego on 27/07/17.
//

#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H

#include <stdbool.h>
#include <stdio.h>

#define SECTION_SIZE 10000
#define NUM_SECTIONS 3

extern long producerWasted;
extern long consumerWasted;

typedef enum{
    notReplicated,
    replicated,
    replicatedHT
}ExecMode;

typedef struct{
    long content[SECTION_SIZE];
    bool isReadMode;
}Section;

typedef struct{
    int enqPtr;
    int deqPtr;
    int enqueueSection;
    int dequeueSection;
    int totalSpaces;
    Section sections[NUM_SECTIONS];
}SectionQueue;

SectionQueue SectionQueue_Init();
void SectionQueue_Enqueue(SectionQueue* this, long value);
long SectionQueue_Dequeue(SectionQueue* this);

void SectionQueue_WastedInst();

#endif //TSXPROJECT_SIMPLEQUEUE_H
