#include "iterator_helper.h"

#include "recomputils.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"

REPY_IteratorHelperInternal* REPY_IteratorHelperInternal_Create(REPY_Handle py_object, REPY_Handle py_scope, const char* var_name) {
    REPY_IteratorHelperInternal* helper = recomp_alloc(sizeof(REPY_IteratorHelperInternal));
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

void REPY_IteratorHelperInternal_Destroy(REPY_IteratorHelperInternal* helper) {
    PythonNative_Release(helper->iter);
    if (PythonNative_IsValidHandle(helper->curr)) {
        // Checking this is important, since curr valid until after the first update, and the iterator COULD be destroyed before then.
        PythonNative_Release(helper->curr);
    }

    if (helper->py_scope != REPY_NO_OBJECT) {
        PythonNative_Release(helper->py_scope);
        PythonNative_Release(helper->var_name);
    }

    recomp_free(helper);
}

bool REPY_IteratorHelperInternal_Update(REPY_IteratorHelperInternal* helper, bool auto_destroy) {
    if (helper->_first_update) {
        helper->_first_update = false;
    } else {
        helper->index++;
        if (helper->curr != 0) {
            PythonNative_Release(helper->curr);
        } else {
            LOGW("Warning: helper->curr should not be 0. You may be trying to update an REPY_IteratorHelperInternal after the iterator is finished.\n");
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
            REPY_IteratorHelperInternal_Destroy(helper);
        }
        return false;   
    }
}

u32 REPY_IteratorHelperInternal_GetIndex(REPY_IteratorHelperInternal* helper) {
    return helper->index;
}

REPY_Handle REPY_IteratorHelperInternal_BorrowCurrent(REPY_IteratorHelperInternal* helper) {
    return helper->curr;
}