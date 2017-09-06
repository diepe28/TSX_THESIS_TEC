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
    int i = 0;
    this.content = (long*) (malloc(sizeof(long) * SIMPLE_SYNC_QUEUE_SIZE));
    for(; i < SIMPLE_SYNC_QUEUE_SIZE; i++){
        this.content[i] = ALREADY_CONSUMED;
    }
    this.enqPtr = this.deqPtr = 0;
    return this;
}

void SimpleSyncQueue_Enqueue(SimpleSyncQueue* this, long value){
    int nextEnqPtr = (this->enqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;

    while(this->content[nextEnqPtr] != ALREADY_CONSUMED)
    {
        asm("pause"); // alone instead of just the busy waiting helps a bit
        //__asm__("HLT");
        //producerCount++;
        //pthread_yield();
        //int lastEnqPtr = this->enqPtr == 0? SIMPLE_SYNC_QUEUE_SIZE -1 : this->enqPtr -1;

        //while(this->content[lastEnqPtr] != ALREADY_CONSUMED)
        //{
            //pthread_yield();
            //pthread_yield();
            //usleep(2);
        //}
    }

    this->content[this->enqPtr] = value;
    this->enqPtr = nextEnqPtr;
    //printf("Producer content[%d]: %ld at time: %ld\n", this->enqPtr -1, value, t1);
}

long SimpleSyncQueue_Dequeue(SimpleSyncQueue* this){
    long value = this->content[this->deqPtr];

    if(value != ALREADY_CONSUMED) {
        this->content[this->deqPtr] = ALREADY_CONSUMED;
        this->deqPtr = (this->deqPtr + 1) % SIMPLE_SYNC_QUEUE_SIZE;
        //consumerCount++;
    }

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