#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "printf.h"
#include "extlib_functions.h"
#include "mod_logging.h"

#include "./api_helpers/if_stmt_helper.h"


#define PYCODE_INLINE_IDENTIFIER_FORMAT "%s in File %s, Function %s, Line %u, Identifier %s -> "

RECOMP_EXPORT char* REPY_InlineCodeSourceStrHelper(char* category, char* filename, char* function_name, u32 line_number, char* identifier) {
    int idlen = lenprintf_(PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    char* retVal = recomp_alloc(idlen + 1);
    snprintf(retVal, idlen + 1, PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    return retVal;
}


RECOMP_EXPORT REPY_IteratorHelper* REPY_IteratorHelper_Create(REPY_Handle py_object, REPY_Handle py_scope, const char* var_name) {
    REPY_IteratorHelper* helper = recomp_alloc(sizeof(REPY_IteratorHelper));
    helper->_first_update = true;
    helper->index = 0;
    helper->iter = PythonNative_Iter(py_object);
    helper->curr = 0;
    helper->py_scope = PythonNative_CopyHandle(py_scope);
    if (py_scope != 0) {
        helper->var_name = PythonNative_CreateStr(var_name);
    }

    return helper;
}

RECOMP_EXPORT void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper) {
    PythonNative_Release(helper->iter);
    if (PythonNative_IsValidHandle(helper->curr)) {
        // Checking this is important, since curr valid until after the first update, and the iterator COULD be destroyed before then.
        PythonNative_Release(helper->curr);
    }

    if (helper->py_scope != 0) {
        PythonNative_Release(helper->py_scope);
        PythonNative_Release(helper->var_name);
    }

    recomp_free(helper);
}

RECOMP_EXPORT bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, bool auto_destroy) {
    if (helper->_first_update) {
        helper->_first_update = false;
    } else {
        helper->index++;
        if (helper->curr != 0) {
            PythonNative_Release(helper->curr);
        } else {
            LOGW("Warning: helper->curr should not be 0. You may be trying to update an REPY_IteratorHelper after the iterator is finished.\n");
        }
    }

    helper->curr = PythonNative_Next(helper->iter, 0, true);
    if (helper->curr) {
        if (helper->py_scope) {
            // If given a python scope, add current to the scope under the given variable name:
            PythonNative_DictSet(helper->py_scope, helper->var_name, helper->curr);
        }

        return true;
    } else {
        // Iterator complete. Time to clean up.
        if (auto_destroy) {
            REPY_IteratorHelper_Destroy(helper);
        }
        return false;   
    }
}

RECOMP_EXPORT REPY_IfStmtChain* REPY_IfStmtChain_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    return (REPY_IfStmtChain*) IfStmtChainInternal_Create(expr_string, filename, function_name, line_number, identifier);
}

RECOMP_EXPORT void REPY_IfStmtChain_Destroy(REPY_IfStmtChain* chain) {
    IfStmtChainInternal_Destroy((REPY_IfStmtChainInternal*) chain);
}

// Helper stuff:
// Initializer values:

RECOMP_EXPORT REPY_IfStmtHelper* REPY_IfStmtHelper_Create(REPY_IfStmtChain** chain_root) {
    return (REPY_IfStmtHelper*) IfStmtHelperInternal_Create((REPY_IfStmtChainInternal**) chain_root);
}

RECOMP_EXPORT void REPY_IfStmtHelper_Destroy(REPY_IfStmtHelper* helper) {
    IfStmtHelperInternal_Destroy((REPY_IfStmtHelperInternal*) helper);
}

RECOMP_EXPORT void REPY_IfStmtHelper_Reset(REPY_IfStmtHelper* p_helper, REPY_IfStmtChain** root) {
    IfStmtHelperInternal_Reset((REPY_IfStmtHelperInternal*)p_helper, (REPY_IfStmtChainInternal**) root);
}

RECOMP_EXPORT bool REPY_IfStmtHelper_Step(REPY_IfStmtHelper* p_helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    return IfStmtHelperInternal_Step((REPY_IfStmtHelperInternal*) p_helper, global_scope, local_scope, expr_string, filename, function_name, line_number, identifier);
};