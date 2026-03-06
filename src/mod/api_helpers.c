#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "printf.h"
#include "extlib_functions.h"
#include "mod_logging.h"

#include "./api_helpers/if_stmt_chain.h"
#include "./api_helpers/if_stmt_helper.h"
#include "./api_helpers/iterator_helper.h"
#include "./api_helpers/helper_auto_cleanup.h"

#define PYCODE_INLINE_IDENTIFIER_FORMAT "%s in File %s, Function %s, Line %u, Identifier %s -> "

RECOMP_EXPORT char* REPY_InlineCodeSourceStrHelper(char* category, char* filename, char* function_name, u32 line_number, char* identifier) {
    int idlen = lenprintf_(PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    char* retVal = recomp_alloc(idlen + 1);
    snprintf(retVal, idlen + 1, PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    return retVal;
}


RECOMP_EXPORT REPY_IteratorHelper* REPY_IteratorHelper_Create(REPY_Handle py_object, REPY_Handle py_scope, const char* var_name) {
    return (REPY_IteratorHelper*) REPY_IteratorHelperInternal_Create(py_object, py_scope, var_name);
}

RECOMP_EXPORT void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper) {
    REPY_IteratorHelperInternal_Destroy((REPY_IteratorHelperInternal*) helper);
}

RECOMP_EXPORT bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, bool auto_destroy) {
    return REPY_IteratorHelperInternal_Update((REPY_IteratorHelperInternal*) helper, auto_destroy);
}

RECOMP_EXPORT u32 REPY_IteratorHelper_GetIndex(REPY_IteratorHelper* helper) {
    return REPY_IteratorHelperInternal_GetIndex((REPY_IteratorHelperInternal*) helper);
}

RECOMP_EXPORT REPY_Handle REPY_IteratorHelper_BorrowCurrent(REPY_IteratorHelper* helper) {
    return REPY_IteratorHelperInternal_BorrowCurrent((REPY_IteratorHelperInternal*) helper);
}


// ==== REPY_IfStmtChain ===
RECOMP_EXPORT REPY_IfStmtChain* REPY_IfStmtChain_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    return (REPY_IfStmtChain*) REPY_IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
}

RECOMP_EXPORT void REPY_IfStmtChain_Destroy(REPY_IfStmtChain* chain) {
    REPY_IfStmtChainInternal_Destroy((REPY_IfStmtChainInternal*) chain);
}

RECOMP_EXPORT REPY_IfStmtChain* REPY_IfStmtChain_BorrowNext(REPY_IfStmtChain* chain) {
    return (REPY_IfStmtChain*) REPY_IfStmtChainInternal_BorrowNext((REPY_IfStmtChainInternal*) chain);
}

RECOMP_EXPORT void REPY_IfStmtChain_StealNext(REPY_IfStmtChain* chain, REPY_IfStmtChain* next) {
    REPY_IfStmtChainInternal_StealNext((REPY_IfStmtChainInternal*) chain, (REPY_IfStmtChainInternal*) next);
}

// Get/Set Bytecode:
RECOMP_EXPORT REPY_Handle REPY_IfStmtChain_BorrowEvalBytecode(REPY_IfStmtChain* chain) {
    return REPY_IfStmtChainInternal_BorrowEvalBytecode((REPY_IfStmtChainInternal*) chain);
}

RECOMP_EXPORT void REPY_IfStmtChain_StealEvalBytecode(REPY_IfStmtChain* chain, REPY_Handle eval_bytecode) {
    REPY_IfStmtChainInternal_StealEvalBytecode((REPY_IfStmtChainInternal*) chain, eval_bytecode);
}


// ==== REPY_IfStmtHelper ===
RECOMP_EXPORT REPY_IfStmtHelper* REPY_IfStmtHelper_Create(REPY_IfStmtChain** chain_root) {
    return (REPY_IfStmtHelper*) REPY_IfStmtHelperInternal_Create((REPY_IfStmtChainInternal**) chain_root);
}

RECOMP_EXPORT void REPY_IfStmtHelper_Destroy(REPY_IfStmtHelper* helper) {
    REPY_IfStmtHelperInternal_Destroy((REPY_IfStmtHelperInternal*) helper);
}

RECOMP_EXPORT bool REPY_IfStmtHelper_Step(REPY_IfStmtHelper* p_helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    return REPY_IfStmtHelperInternal_Step((REPY_IfStmtHelperInternal*) p_helper, global_scope, local_scope, expr_string, filename, function_name, line_number, identifier);
};

// === Helper Auto-Cleanup ===
RECOMP_EXPORT REPY_HelperAutoCleanup* REPY_HelperAutoCleanup_Create() {
    return (REPY_HelperAutoCleanup*) REPY_HelperAutoCleanupInternal_Create();
}

RECOMP_EXPORT REPY_IteratorHelper* REPY_HelperAutoCleanup_AddIteratorHelper(REPY_HelperAutoCleanup* cleanup, REPY_IteratorHelper* iterator_helper) {
    return (REPY_IteratorHelper*) REPY_HelperAutoCleanupInternal_AddIteratorHelper((REPY_HelperAutoCleanupInternal*)cleanup, (REPY_IteratorHelperInternal*)iterator_helper);
}

RECOMP_EXPORT REPY_IfStmtHelper* REPY_HelperAutoCleanup_AddIfStmtHelper(REPY_HelperAutoCleanup* cleanup, REPY_IfStmtHelper* if_stmt_helper) {
    return (REPY_IfStmtHelper*) REPY_HelperAutoCleanupInternal_AddIfStmtHelper((REPY_HelperAutoCleanupInternal*) cleanup, (REPY_IfStmtHelperInternal*) if_stmt_helper);
}
RECOMP_EXPORT void REPY_HelperAutoCleanup_CleanNow(REPY_HelperAutoCleanup* cleanup) {
    REPY_HelperAutoCleanupInternal_CleanNow((REPY_HelperAutoCleanupInternal*) cleanup);
}

RECOMP_EXPORT void REPY_HelperAutoCleanup_Destroy(REPY_HelperAutoCleanup* cleanup, bool clean_now) {
    REPY_HelperAutoCleanupInternal_Destroy((REPY_HelperAutoCleanupInternal*) cleanup, clean_now);
}