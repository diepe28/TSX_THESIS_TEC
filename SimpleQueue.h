//
// Created by diego on 27/07/17.
//

#ifndef TSXPROJECT_SIMPLEQUEUE_H
#define TSXPROJECT_SIMPLEQUEUE_H
#include <stdlib.h>
#include <stdbool.h>

#define SECTION_SIZE 10
#define NUM_SECTIONS 2

typedef struct{
    int content[SECTION_SIZE];
    bool isReadMode;
}Section;

Section Section_Init();
//void Section_Destroy(Section* this);
void Section_Enqueu(Section this, int value, int index);

typedef struct{
    int enqPtr;
    int deqPtr;
    int totalSpaces;
    Section sections[NUM_SECTIONS];
}SimpleQueue;

SimpleQueue SimpleQueue_Init();
//void SimpleQueue_Destroy(SimpleQueue this);
void SimpleQueue_Enqueue(SimpleQueue* this, int value);
int SimpleQueue_Dequeue(SimpleQueue* this);

#endif //TSXPROJECT_SIMPLEQUEUE_H
