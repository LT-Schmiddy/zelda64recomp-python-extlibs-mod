#include "if_stmt_helper.h"

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "../printf.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"



// If Statement Helper
REPY_IfStmtChainInternal* IfStmtChainInternal_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    REPY_IfStmtChainInternal* retVal = recomp_alloc(sizeof(REPY_IfStmtChainInternal));
    char* id_str = REPY_InlineCodeSourceStrHelper("IfStmtHelper", filename, function_name, line_number, identifier);
    retVal->eval_expression_bytecode = PythonNative_CompileCStr(expr_string, id_str, REPY_CODE_EVAL);
    recomp_free(id_str);
    retVal->next = NULL;

    return retVal;
}

void IfStmtChainInternal_Destroy(REPY_IfStmtChainInternal* chain) {
    PythonNative_Release(chain->eval_expression_bytecode);
    // Destroy helper chain recursively.
    if (chain->next != NULL) {
        IfStmtChainInternal_Destroy(chain->next);
    }
    recomp_free(chain);
}

// Helper stuff:
void IfStmtHelperInternal_Reset(REPY_IfStmtHelperInternal* helper, REPY_IfStmtChainInternal** root) {
    helper->index = 0;
    helper->root = root;
    helper->curr = NULL;
    helper->_first_step = true;
}

REPY_IfStmtHelperInternal* IfStmtHelperInternal_Create(REPY_IfStmtChainInternal** chain_root) {
    REPY_IfStmtHelperInternal* retVal = recomp_alloc(sizeof(REPY_IfStmtHelperInternal));

    IfStmtHelperInternal_Reset(retVal, chain_root);

    return retVal;
}

void IfStmtHelperInternal_Destroy(REPY_IfStmtHelperInternal* helper) {
    recomp_free(helper);
}


bool IfStmtHelperInternal_Step(REPY_IfStmtHelperInternal* helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    // First time setup of the helper:
    if (helper->_first_step) {
        helper->_first_step = false;
        
        if (*(helper->root) == NULL) {
            // No chain was ever created. Starting one now.
            (*(helper->root)) = IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
        }
        helper->curr = (*(helper->root));
    } else {
        if (helper->curr->next == NULL) {
            // The next link in the clain doesn't exist. Let's create it.
            helper->curr->next = IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
        }
        // Move down the chain:
        helper->curr = helper->curr->next;
        helper->index++;
    }
    return PythonNative_CastBool(PythonNative_MakeSUH(PythonNative_Eval(helper->curr->eval_expression_bytecode, global_scope, local_scope)));
};