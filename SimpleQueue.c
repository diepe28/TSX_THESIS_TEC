//
// Created by diego on 27/07/17.
//

#include "SimpleQueue.h"




SimpleQueue SimpleQueue_Init(){
    int i;
    SimpleQueue this;
    this.deqPtr = this.enqPtr = 0;
    this.totalSpaces = NUM_SECTIONS * SECTION_SIZE;

    for(i = 0; i < NUM_SECTIONS; i++){
        this.sections[i].isReadMode = 0;
    }

    return this;
}

void SimpleQueue_Enqueue(SimpleQueue* this, int value){
    int enqueueSection = this->enqPtr / SECTION_SIZE;
    int sectionIndex = this->enqPtr % SECTION_SIZE;

    while(this->sections[enqueueSection].isReadMode); // the consumer thread is using such section

    this->sections[enqueueSection].content[sectionIndex] = value;
    if(sectionIndex+1 == SECTION_SIZE)
        this->sections[enqueueSection].isReadMode = 1;

    this->enqPtr = (this->enqPtr +1) % this->totalSpaces;
}

int SimpleQueue_Dequeue(SimpleQueue* this){
    int enqueSection = this->deqPtr / SECTION_SIZE;
    int sectionIndex = this->deqPtr % SECTION_SIZE;

    while(this->sections[enqueSection].isReadMode); // the consumer thread is using such section

    if(sectionIndex+1 == SECTION_SIZE)
        this->sections[enqueSection].isReadMode = 0;

    int value = this->sections[enqueSection].content[sectionIndex];

    this->deqPtr = (this->deqPtr +1) % this->totalSpaces;

    return value;
}