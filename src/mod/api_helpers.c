#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"

#include "extlib_functions.h"

RECOMP_EXPORT u32 REPY_CompileHelper (
    REPY_Handle* handle_ptr, 
    const char* code_str, 
    const char* identifier,
    REPY_CodeMode code_mode, 
    REPY_CHReturnType return_type
) {
    recomp_printf("Running REPY_CompileHelper\n");
    bool was_compiled = false;
    if (*handle_ptr == 0) {
        *handle_ptr = REPY_CompileCStr(code_str, identifier, code_mode);
        recomp_printf("REPY_CompileHelper compiled handle %u\n", *handle_ptr);
        bool was_compiled = true;
    } 

    // Handling Return:
    switch(return_type) {
        case REPY_RETURN_TRUE:
            return true;
        case REPY_RETURN_WAS_COMPILED:
            return was_compiled;
        case REPY_RETURN_WAS_COMPILED_SUCCESSFULLY:
            return was_compiled && *handle_ptr;
        case REPY_RETURN_HANDLE:
            return *handle_ptr;
        case REPY_RETURN_FALSE:
        default:
            return false;
    }
}

RECOMP_EXPORT REPY_IteratorHelper* REPY_IteratorHelper_Init(REPY_Handle py_object) {
    REPY_IteratorHelper* helper = recomp_alloc(sizeof(REPY_IteratorHelper));
    helper->_first_update = true;
    helper->index = 0;
    helper->iter = PythonNative_Object_Iter(py_object);
    helper->curr = 0;

    return helper;
}

RECOMP_EXPORT bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, REPY_Handle py_scope, const char* var_name) {
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

    helper->curr = PythonNative_Object_Next(helper->iter, 0, false);

    if (helper->curr) {
        if (py_scope) {
            // If given a python scope, add current to the scope under the given variable name:
            PythonNative_Dict_Set(py_scope, PythonNative_Object_MakeSUH(PythonNative_Object_CreateStr(var_name)), helper->curr);
        }

        return true;
    } else {
        // Iterator complete. Time to clean up.
        PythonNative_Object_Release(helper->iter);
        recomp_free(helper);
        return false;   
    }
}