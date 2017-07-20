//
// Created by diego on 19/07/17.
//

#ifndef TSXPROJECT_TESTCOREAFFINITY_H
#define TSXPROJECT_TESTCOREAFFINITY_H

typedef enum {
    normal,
    normallyReplicated,
    normallyReplicatedWithHT,
    hyperReplicated
} ExecutionType;

void CoreAffinity_View(int numThreads, int useHyperThread);

void CoreAffinity_Replication_Test(ExecutionType execType);

#endif //TSXPROJECT_TESTCOREAFFINITY_H
