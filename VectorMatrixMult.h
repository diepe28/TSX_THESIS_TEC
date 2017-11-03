//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
//#include <glib.h>

#define MATRIX_COLS 2900
#define MATRIX_ROWS 2900

int vector[MATRIX_COLS];
long producerVectorResult[MATRIX_ROWS];
long consumerVectorResult[MATRIX_ROWS];
int **matrix;

void Vector_Matrix_Init();
void Vector_Matrix_Mult(int producerCore, int consumerCore, int producerCoreHT, int consumerCoreHT);

void printMatrix(int rows, int cols, long matrix[rows][cols]);

#endif //TSXPROJECT_TEST_HYPERTHREAD_H
