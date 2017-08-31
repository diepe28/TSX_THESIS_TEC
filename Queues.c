//
// Created by diego on 27/07/17.
//

#include <time.h>
#include "Queues.h"

void SetThreadAffinity(int threadId) {
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(threadId, &cpuset);

    /* pin the thread to a core */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        fprintf(stderr, "Thread pinning failed!\n");
        exit(1);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Multi Section Queue ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

long volatile producerCount = 0;
long volatile consumerCount = 0;

SectionQueue SectionQueue_Init(){
    int i;
    SectionQueue this;
    this.deqPtr = this.enqPtr = this.enqueueSection = this.dequeueSection = 0;

    for(i = 0; i < NUM_SECTIONS; i++){
        this.sections[i].isReadMode = false;
    }

    return this;
}


void inline SectionQueue_Enqueue(SectionQueue* this, long value){
    //printf("Producer thread section: %d, index: %d, isReadMode: %d \n", enqueueSection, sectionIndex, this->sections[enqueueSection].isReadMode);
    while(this->sections[this->enqueueSection].isReadMode); // producerWasted++;

    this->sections[this->enqueueSection].content[this->enqPtr] = value;
    this->enqPtr = (this->enqPtr +1) % SECTION_SIZE;

    if(this->enqPtr == 0) {
        this->sections[this->enqueueSection].isReadMode = 1;
        this->enqueueSection = (this->enqueueSection + 1) % NUM_SECTIONS;
    }
    //producerCount++; // to avoid weird behavior due optimizations
}

long inline SectionQueue_Dequeue(SectionQueue* this){
    long value;

    //printf("Consumer thread section: %d, index: %d, isReadMode: %d \n", dequeueSection, sectionIndex, this->sections[dequeueSection].isReadMode);
    while(!this->sections[this->dequeueSection].isReadMode); // consumerWasted++;

    value = this->sections[this->dequeueSection].content[this->deqPtr];
    this->deqPtr = (this->deqPtr +1) % SECTION_SIZE;

    if(this->deqPtr == 0) {
        this->sections[this->dequeueSection].isReadMode = 0;
        this->dequeueSection = (this->dequeueSection + 1) % NUM_SECTIONS;
    }
    //consumerCount++; // to avoid weird behavior due optimizations
    return value;
}

void SectionQueue_WastedInst(){
    printf("Consumer wasted %ld, producer wasted: %ld \n", consumerCount, producerCount);
}
//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Simple Queue ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

SimpleQueue SimpleQueue_Init(){
    SimpleQueue this;
    this.enqPtr = this.deqPtr = 0;
    return this;
}
void inline SimpleQueue_Enqueue(SimpleQueue* this, long value){
    //printf("Producer enqPtr: %d\n", this->enqPtr);
    int nextEnqPtr = (this->enqPtr + 1) % SIMPLE_QUEUE_MAX_ELEMENTS;
    while(nextEnqPtr == this->deqPtr);
    this->content[this->enqPtr] = value;
    this->enqPtr = nextEnqPtr;
    //producerCount++; // to avoid weird behavior due optimizations
}

long inline SimpleQueue_Dequeue(SimpleQueue* this){
    //printf("Consumer deqPtr: %d\n", this->deqPtr);
    while(this->deqPtr == this->enqPtr);
    long value = this->content[this->deqPtr];
    this->deqPtr = (this->deqPtr + 1) % SIMPLE_QUEUE_MAX_ELEMENTS;
    //consumerCount++; // to avoid weird behavior due optimizations
    return value;
}

//////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Simple Sync Queue /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

SimpleSyncQueue SimpleSyncQueue_Init(){
    SimpleSyncQueue this;
    this.content = (long*) (malloc(sizeof(long) * SIMPLE_SYNC_QUEUE_SIZE));
    this.enqPtr = this.deqPtr;
    return this;
}

void SimpleSyncQueue_Enqueue(SimpleSyncQueue* this, long value){
    int nextEnqPtr = (this->enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
    while(nextEnqPtr == this->deqPtr);
        //sleep(1);

    this->content[this->enqPtr] = value;
    clock_t t1 = clock();
    this->enqPtr = nextEnqPtr;
    //producerCount++;
    //printf("Producer content[%d]: %ld at time: %ld\n", this->enqPtr -1, value, t1);
}

long SimpleSyncQueue_Dequeue(SimpleSyncQueue* this){

    long value = this->content[this->deqPtr];
    this->content[this->deqPtr] = -2;
    clock_t t1 = clock();

    this->deqPtr = (this->deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
    //consumerCount++;
    //printf("Consumer content[%d]: %ld at time: %ld\n", this->deqPtr -1, value, t1);
    return value;
}

SimpleSyncQueue SimpleSyncQueue_Destroy(SimpleSyncQueue* this){
    if(this->content)
        free(this->content);
}

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// GLOBAL FUNCS FOR QUEUES ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void Global_SetQueueType(QueueType newQueueType){
    queueType = newQueueType;
}
void Global_SetCheckFrequency(CheckFrequency newCheckFrequency){
    checkFrequency = newCheckFrequency;
}