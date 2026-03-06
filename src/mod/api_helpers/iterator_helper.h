#ifndef __ITERATOR_HELPER__
#define __ITERATOR_HELPER__

#include "repy_api.h"
#include "global.h"

typedef struct REPY_IteratorHelperInternal {
    REPY_Handle iter; ///< Handle for the Python iterator object. 
    u32 index; ///< The index of the current object from the iterator.
    REPY_Handle curr; ///< Handle of the current object from the iterator. If you need to access this object outside of the current iteration, use `REPY_CopyHandle` to get a new handle.
    REPY_Handle py_scope; ///< If this handle != 0, then the `curr` object will be added to this scope with a variable name set by `var_name`.
    REPY_Handle var_name; ///< The variable name that will be used for `curr` when added to `py_scope`, if `py_scope` is not 0.
    bool _first_update; ///< Internal flag used to determine if the iterator has been updated for the first time.
    bool auto_destroy; /// Should self-deallocate on final update
} REPY_IteratorHelperInternal;

REPY_IteratorHelperInternal* REPY_IteratorHelperInternal_Create(REPY_Handle py_object, REPY_Handle py_scope_nullable, const char* var_name, bool auto_destroy);
void REPY_IteratorHelperInternal_Destroy(REPY_IteratorHelperInternal* helper);
bool REPY_IteratorHelperInternal_Update(REPY_IteratorHelperInternal* helper);

u32 REPY_IteratorHelperInternal_GetIndex(REPY_IteratorHelperInternal* helper);
REPY_Handle REPY_IteratorHelperInternal_BorrowCurrent(REPY_IteratorHelperInternal* helper);

#endif