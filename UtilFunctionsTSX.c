#include "UtilFunctionsTSX.h"

const int TSX_EXPLICIT_ABORT_CODE = 0xff;

///
/// Tries to get the reason message why a transaction failed.
/// Param code: the value of the error.
///
char * TSX_GetTransactionAbortMessage(int transactionStatus) {
    char *message = (char*) malloc((sizeof(char) * 100));

    strcpy(message, "Transaction failed due to unknown reason");

    if(TSX_WasExplicitAbort(transactionStatus))
        strcpy(message, "Transaction explicitly aborted with _xabort");
    else if(transactionStatus &  _XABORT_RETRY)
        strcpy(message, "Transaction retry is possible.");
    else if(transactionStatus & _XABORT_CONFLICT)
        strcpy(message, "Transaction abort due to a memory conflict with another thread");
    else if(transactionStatus &  _XABORT_CAPACITY)
        strcpy(message, "Transaction abort due to the transaction using too much memory");
    else if(transactionStatus &  _XABORT_DEBUG)
        strcpy(message, "Transaction abort due to a debug trap");
    else if(transactionStatus &  _XABORT_NESTED)
        strcpy(message, "Transaction abort in a inner nested transaction");

    return message;
}

/// Returns true if the transaction status has the _XABORT_EXPLICIT bit flag and the  _XABORT_CODE
/// returns the same TSX_EXPLICIT_ABORT_CODE used in _xabort(0xff)
/// \param transactionStatus the status returned by _xbegin() or _xabort()
/// \return 1 if both conditions are met, 0 otherwise
int TSX_WasExplicitAbort(int transactionStatus){
    return (transactionStatus & _XABORT_EXPLICIT) && (_XABORT_CODE(transactionStatus) == TSX_EXPLICIT_ABORT_CODE);
}

/// Returns 1 if the transaction status has the _XABORT_RETRY bit flag on.
/// \param transactionStatus the status returned by _xbegin() or _xabort()
/// \return true if retry is possible, 0 otherwise
int TSX_IsRetryPossible(int transactionStatus){
    return (transactionStatus & _XABORT_RETRY) != 0;
}

unsigned int TSX_Get_Explicit_Abort_Code(){
    return TSX_EXPLICIT_ABORT_CODE;
}