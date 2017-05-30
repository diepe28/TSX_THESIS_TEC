#ifndef UTIL_FUNCTIONS_TSX_H
#define UTIL_FUNCTIONS_TSX_H
#include "rtm.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern const int TSX_EXPLICIT_ABORT_CODE;

char * TSX_GetTransactionAbortMessage(int transactionStatus);
int TSX_WasExplicitAbort(int transactionStatus);
int TSX_IsRetryPossible(int transactionStatus);
unsigned int TSX_Get_Explicit_Abort_Code();

#endif //UTIL_FUNCTIONS_TSX_H
