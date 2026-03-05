#include "if_stmt_helper.h"

#include "global.h"
#include "recomputils.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"

REPY_IfStmtHelperInternal* IfStmtHelperInternal_Create(REPY_IfStmtChainInternal** chain_root) {
    REPY_IfStmtHelperInternal* helper = recomp_alloc(sizeof(REPY_IfStmtHelperInternal));

    helper->index = 0;
    helper->root = chain_root;
    helper->curr = NULL;
    helper->_first_step = true;

    return helper;
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
            (*(helper->root)) = REPY_IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
        }
        helper->curr = (*(helper->root));
    } else {
        if (helper->curr->next == NULL) {
            // The next link in the clain doesn't exist. Let's create it.
            helper->curr->next = REPY_IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
        }
        // Move down the chain:
        helper->curr = helper->curr->next;
        helper->index++;
    }
    return PythonNative_CastBool(PythonNative_MakeSUH(PythonNative_Eval(helper->curr->eval_expression_bytecode, global_scope, local_scope)));
};