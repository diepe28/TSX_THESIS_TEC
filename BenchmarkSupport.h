//
// Created by dperez on 22/05/17.
//

#ifndef TSXEXAMPLE_BENCHMARKSUPPORT_H
#define TSXEXAMPLE_BENCHMARKSUPPORT_H

#include <glib.h>
#include <gtk/gtk.h>
#include "TestTSX.h"

void BENCHMARK_SUPPORT_EvaluateTransactions(int numExecutions, int numThread, int usingTM);

#endif //TSXEXAMPLE_BENCHMARKSUPPORT_H
