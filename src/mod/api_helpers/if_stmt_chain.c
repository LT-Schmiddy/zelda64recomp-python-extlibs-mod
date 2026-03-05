#include "if_stmt_chain.h"

#include "global.h"
#include "recomputils.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"

REPY_IfStmtChainInternal* REPY_IfStmtChainInternal_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    REPY_IfStmtChainInternal* retVal = recomp_alloc(sizeof(REPY_IfStmtChainInternal));
    char* id_str = REPY_InlineCodeSourceStrHelper("IfStmtHelper", filename, function_name, line_number, identifier);
    retVal->eval_expression_bytecode = PythonNative_CompileCStr(expr_string, id_str, REPY_CODE_EVAL);
    recomp_free(id_str);
    retVal->next = NULL;

    return retVal;
}

void REPY_IfStmtChainInternal_Destroy(REPY_IfStmtChainInternal* chain) {
    PythonNative_Release(chain->eval_expression_bytecode);
    // Destroy helper chain recursively.
    if (chain->next != NULL) {
        REPY_IfStmtChainInternal_Destroy(chain->next);
    }
    recomp_free(chain);
}

// Get/Set Next Link:
REPY_IfStmtChainInternal* REPY_IfStmtChainInternal_BorrowNext(REPY_IfStmtChainInternal* chain) {
    return chain->next;
}

void REPY_IfStmtChainInternal_StealNext(REPY_IfStmtChainInternal* chain, REPY_IfStmtChainInternal* next) {
    chain->next = next;
}

// Get/Set Bytecode:
REPY_Handle REPY_IfStmtChainInternal_BorrowEvalBytecode(REPY_IfStmtChainInternal* chain) {
    return chain->eval_expression_bytecode;
}

void REPY_IfStmtChainInternal_StealEvalBytecode(REPY_IfStmtChainInternal* chain, REPY_Handle eval_bytecode) {
    chain->eval_expression_bytecode = eval_bytecode;
}

