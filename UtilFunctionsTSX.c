#include "UtilFunctionsTSX.h"

///
/// Tries to get the reason message why a transaction failed.
/// Param code: the value of the error.
///
char * TSX_GetTransactionAbortMessage(int code) {
    char *message = (char*) malloc((sizeof(char) * 100));

    if(code & _XABORT_EXPLICIT)
        strcpy(message, "Transaction explicitly aborted with _xabort");
    if(code &  _XABORT_RETRY)
        strcpy(message, "Transaction retry is possible.");
    if(code & _XABORT_CONFLICT)
        strcpy(message, "Transaction abort due to a memory conflict with another thread");
    if(code &  _XABORT_CAPACITY)
        strcpy(message, "Transaction abort due to the transaction using too much memory");
    if(code &  _XABORT_DEBUG)
        strcpy(message, "Transaction abort due to a debug trap");
    if(code &  _XABORT_NESTED)
        strcpy(message, "Transaction abort in a inner nested transaction");

    return message;
}