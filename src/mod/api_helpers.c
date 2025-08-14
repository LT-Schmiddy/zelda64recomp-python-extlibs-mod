#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"
#include "printf.h"
#include "extlib_functions.h"
#include "mod_logging.h"
#include "mod_logging.h"


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
    helper->iter = PythonNative_Object_Iter(py_object);
    helper->curr = 0;
    helper->py_scope = PythonNative_Object_CopyHandle(py_scope);
    if (py_scope != 0) {
        helper->var_name = PythonNative_Object_CreateStr(var_name);
    }

    
    return helper;
}

RECOMP_EXPORT void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper) {
    PythonNative_Object_Release(helper->iter);
    if (PythonNative_Object_IsValidHandle(helper->curr)) {
        PythonNative_Object_Release(helper->curr);
    }

    if (helper->py_scope != 0) {
        PythonNative_Object_Release(helper->py_scope);
        PythonNative_Object_Release(helper->var_name);
    }

    recomp_free(helper);
}

RECOMP_EXPORT bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, bool auto_destroy) {
    if (helper->_first_update) {
        helper->_first_update = false;
    } else {
        helper->index++;
        if (helper->curr != 0) {
            PythonNative_Object_Release(helper->curr);
        } else {
            LOGW("Warning: helper->curr should not be 0. You may be trying to update an REPY_IteratorHelper after the iterator is finished.\n");
        }
    }

    helper->curr = PythonNative_Object_Next(helper->iter, 0, true);
    if (helper->curr) {
        if (helper->py_scope) {
            // If given a python scope, add current to the scope under the given variable name:
            PythonNative_Dict_Set(helper->py_scope, helper->var_name, helper->curr);
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



// If Statement Helper -
RECOMP_EXPORT REPY_IfStmtChain* REPY_IfStmtChain_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    REPY_IfStmtChain* retVal = recomp_alloc(sizeof(REPY_IfStmtChain));
    char* id_str = REPY_InlineCodeSourceStrHelper("IfStmtHelper", filename, function_name, line_number, identifier);
    retVal->eval_expression_bytecode = PythonNative_CompileCStr(expr_string, id_str, REPY_CODE_EVAL);
    recomp_free(id_str);
    retVal->next = NULL;

    return retVal;
}

RECOMP_EXPORT void REPY_IfStmtChain_Destroy(REPY_IfStmtChain* chain) {
    // Destroy helper chain recursively.
    if (chain->next != NULL) {
        REPY_IfStmtChain_Destroy(chain->next);
    }
    PythonNative_Object_Release(chain->eval_expression_bytecode);
    recomp_free(chain);
}

RECOMP_EXPORT void REPY_IfStmtHelper_InitInPlace(REPY_IfStmtHelper* helper, REPY_IfStmtChain** root) {
    helper->index = 0;
    helper->root = root;
    helper->curr = NULL;
    helper->_first_step = true;
}

RECOMP_EXPORT bool REPY_IfStmtHelper_Step(REPY_IfStmtHelper* helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier) {
    // First time setup of the helper:
    if (helper->_first_step) {
        helper->_first_step = false;
        
        if (*(helper->root) == NULL) {
            // No chain was ever created. Starting one now.
            (*(helper->root)) = REPY_IfStmtChain_Create(expr_string, filename, function_name, line_number, identifier);
        }
        helper->curr = (*(helper->root));
    } else {
        if (helper->curr->next == NULL) {
            // The next link in the clain doesn't exist. Let's create it.
            helper->curr->next = REPY_IfStmtChain_Create(expr_string, filename, function_name, line_number, identifier);
        }
        // Move down the chain:
        helper->curr = helper->curr->next;
        helper->index++;
    }
    return PythonNative_Object_CastBool(PythonNative_Object_MakeSUH(PythonNative_Eval(helper->curr->eval_expression_bytecode, global_scope, local_scope)));
};


