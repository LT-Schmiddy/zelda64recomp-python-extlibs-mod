#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"
#include "printf.h"
#include "extlib_functions.h"

RECOMP_EXPORT u32 REPY_CompileHelper (
    REPY_Handle* handle_ptr, 
    const char* code_str, 
    const char* identifier,
    REPY_CodeMode code_mode, 
    REPY_CHReturnType return_type
) {
    bool was_compiled = false;
    if (*handle_ptr == 0) {
        *handle_ptr = REPY_CompileCStr(code_str, identifier, code_mode);
        bool was_compiled = true;
    } 

    // Handling Return:
    switch(return_type) {
        case REPY_CH_RETURN_TRUE:
            return true;
        case REPY_CH_RETURN_WAS_COMPILED:
            return was_compiled;
        case REPY_CH_RETURN_WAS_COMPILED_SUCCESSFULLY:
            return was_compiled && *handle_ptr;
        case REPY_CH_RETURN_HANDLE:
            return *handle_ptr;
        case REPY_CH_RETURN_FALSE:
        default:
            return false;
    }
}

RECOMP_EXPORT REPY_IteratorHelper* REPY_IteratorHelper_Create(REPY_Handle py_object, REPY_Handle py_scope, const char* var_name) {
    REPY_IteratorHelper* helper = recomp_alloc(sizeof(REPY_IteratorHelper));
    helper->_first_update = true;
    helper->index = 0;
    helper->iter = PythonNative_Object_Iter(py_object);
    helper->curr = 0;
    helper->py_scope = PythonNative_Object_CopyHandle(py_scope);
    helper->var_name = PythonNative_Object_CreateStr(var_name);
    
    return helper;
}

RECOMP_EXPORT void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper) {
    PythonNative_Object_Release(helper->iter);
    PythonNative_Object_Release(helper->py_scope);
    PythonNative_Object_Release(helper->var_name);
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
            recomp_printf("Warning: helper->curr should be 0. You may be trying to update an REPY_IteratorHelper after the iterator is finished.\n");
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

#define PYCODE_INLINE_IDENTIFIER_FORMAT "%s in File %s, Function %s, Line %u, Identifier %s -> "
RECOMP_EXPORT char* REPY_InlineCodeSourceStrHelper(char* category, char* filename, char* function_name, u32 line_number, char* identifier) {
    int idlen = lenprintf_(PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    char* retVal = recomp_alloc(idlen + 1);
    snprintf(retVal, idlen + 1, PYCODE_INLINE_IDENTIFIER_FORMAT, category, filename, function_name, line_number,identifier);
    return retVal;
}

