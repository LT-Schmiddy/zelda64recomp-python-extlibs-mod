#ifndef __IF_STMT_CHAIN__
#define __IF_STMT_CHAIN__

#include "repy_api.h"

// If Statement chains used to help with REPY_FN. Uses object-oriented design.

// Main Struct:
typedef struct REPY_IfStmtChainInternal {
    REPY_Handle eval_expression_bytecode; ///< The bytecode for the Python expression to evaluate.
    struct REPY_IfStmtChainInternal* next; ///< Pointer to the next link in the chain.
} REPY_IfStmtChainInternal;

REPY_IfStmtChainInternal* REPY_IfStmtChainInternal_Create(char* expr_string, char* filename, char* function_name, REPY_u32 line_number, char* identifier);
void REPY_IfStmtChainInternal_Destroy(REPY_IfStmtChainInternal* chain);

// Get/Set Next Link:
REPY_IfStmtChainInternal* REPY_IfStmtChainInternal_BorrowNext(REPY_IfStmtChainInternal* chain);
void REPY_IfStmtChainInternal_StealNext(REPY_IfStmtChainInternal* chain, REPY_IfStmtChainInternal* next);

// Get/Set Bytecode:
REPY_Handle REPY_IfStmtChainInternal_BorrowEvalBytecode(REPY_IfStmtChainInternal* chain);
void REPY_IfStmtChainInternal_StealEvalBytecode(REPY_IfStmtChainInternal* chain, REPY_Handle eval_bytecode);

#endif