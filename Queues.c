//
// Created by diego on 27/07/17.
//

#include "Queues.h"

long producerWasted = 0;
long consumerWasted = 0;

SectionQueue SectionQueue_Init(){
    int i;
    SectionQueue this;
    this.deqPtr = this.enqPtr = this.enqueueSection = this.dequeueSection = 0;

    for(i = 0; i < NUM_SECTIONS; i++){
        this.sections[i].isReadMode = false;
    }

    return this;
}

void SectionQueue_Enqueue(SectionQueue* this, long value){
    //printf("Producer thread section: %d, index: %d, isReadMode: %d \n", enqueueSection, sectionIndex, this->sections[enqueueSection].isReadMode);
    while(this->sections[this->enqueueSection].isReadMode); // producerWasted++;

    this->sections[this->enqueueSection].content[this->enqPtr] = value;
    this->enqPtr = (this->enqPtr +1) % SECTION_SIZE;

    if(this->enqPtr == 0) {
        this->sections[this->enqueueSection].isReadMode = 1;
        this->enqueueSection = (this->enqueueSection + 1) % NUM_SECTIONS;
    }
}

long SectionQueue_Dequeue(SectionQueue* this){
    long value;

    //printf("Consumer thread section: %d, index: %d, isReadMode: %d \n", dequeueSection, sectionIndex, this->sections[dequeueSection].isReadMode);
    while(!this->sections[this->dequeueSection].isReadMode); // consumerWasted++;

    value = this->sections[this->dequeueSection].content[this->deqPtr];
    this->deqPtr = (this->deqPtr +1) % SECTION_SIZE;

    if(this->deqPtr == 0) {
        this->sections[this->dequeueSection].isReadMode = 0;
        this->dequeueSection = (this->dequeueSection + 1) % NUM_SECTIONS;
    }

    return value;
}

void SectionQueue_WastedInst(){
    printf("Consumer wasted %ld, producer wasted: %ld \n", consumerWasted, producerWasted);
}