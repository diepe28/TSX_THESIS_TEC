//
// Created by diego on 26/07/17.
//

#ifndef TSXPROJECT_TEST_HYPERTHREAD_H
#define TSXPROJECT_TEST_HYPERTHREAD_H

#define _GNU_SOURCE
#include "TestTSX.h"
#include "Queues.h"
#include <glib.h>

#define MATRIX_COLS 20000
#define MATRIX_ROWS 20000

//#define MATRIX_COLS 100
//#define MATRIX_ROWS 100

void Vector_Matrix_Mult(int useHyperThread);
#endif //TSXPROJECT_TEST_HYPERTHREAD_H
