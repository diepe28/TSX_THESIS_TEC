//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

#define MATRIX_COLS 10000
#define MATRIX_ROWS 10000

//#define MATRIX_COLS 100
//#define MATRIX_ROWS 100

int vector[MATRIX_COLS];
long producerVectorResult[MATRIX_ROWS];
long consumerVectorResult[MATRIX_ROWS];
int **matrix;

void Vector_Matrix_Init();
void Vector_Matrix_Mult(int useHyperThread);

void printMatrix(int rows, int cols, long matrix[rows][cols]);

#endif //TSXPROJECT_TEST_HYPERTHREAD_H
