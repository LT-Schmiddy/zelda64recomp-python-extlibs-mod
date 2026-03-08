#include "deferred_cleanup_helper.h"

#include "recomputils.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"

REPY_DeferredCleanupHelperInternal* REPY_DeferredCleanupHelperInternal_Create() {
    REPY_DeferredCleanupHelperInternal* retVal = recomp_alloc(sizeof(REPY_DeferredCleanupHelperInternal));
    retVal->chain_start = NULL;

    return retVal;
}

REPY_Handle REPY_DeferredCleanupHelperInternal_AddHandle(REPY_DeferredCleanupHelperInternal* cleanup, REPY_Handle handle) {
    // Create a new link and configure it:
    REPY_AutoCleanupHelperInternal_Link* new_link = recomp_alloc(sizeof(REPY_AutoCleanupHelperInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_HANDLE;
    new_link->handle = handle;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("REPY_Handle at 0x%08X queued for auto-cleanup.", handle);
    return handle;
}

void* REPY_DeferredCleanupHelperInternal_AddRecompFree(REPY_DeferredCleanupHelperInternal* cleanup, void* pointer) {
    // Create a new link and configure it:
    REPY_AutoCleanupHelperInternal_Link* new_link = recomp_alloc(sizeof(REPY_AutoCleanupHelperInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_RECOMP_FREE;
    new_link->pointer = (void*) pointer;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("Pointer (recomp_alloc/free) 0x%p queued for auto-cleanup.", pointer);
    return pointer;
}

REPY_IteratorHelperInternal* REPY_DeferredCleanupHelperInternal_AddIteratorHelper(REPY_DeferredCleanupHelperInternal* cleanup, REPY_IteratorHelperInternal* iterator_helper) {
    // Create a new link and configure it:
    REPY_AutoCleanupHelperInternal_Link* new_link = recomp_alloc(sizeof(REPY_AutoCleanupHelperInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_ITERATORHELPER;
    new_link->pointer = (void*) iterator_helper;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("REPY_IteratorHelper at 0x%p queued for auto-cleanup.", iterator_helper);
    return iterator_helper;
}

REPY_IfStmtHelperInternal* REPY_DeferredCleanupHelperInternal_AddIfStmtHelper(REPY_DeferredCleanupHelperInternal* cleanup, REPY_IfStmtHelperInternal* if_stmt_helper) {
    // Create a new link and configure it:
    REPY_AutoCleanupHelperInternal_Link* new_link = recomp_alloc(sizeof(REPY_AutoCleanupHelperInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_IFSTMTHELPER;
    new_link->pointer = (void*) if_stmt_helper;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("REPY_IfStmtHelper at 0x%p queued for auto-cleanup.", if_stmt_helper);
    return if_stmt_helper;
}

void REPY_DeferredCleanupHelperInternal_CleanNow(REPY_DeferredCleanupHelperInternal* cleanup) {
    // Traverse the chain, destroying each helper as we go:
    REPY_AutoCleanupHelperInternal_Link* this_link = cleanup->chain_start;

    while(this_link != NULL) {
        switch (this_link->cleanup_type) {
            case REPY_AUTOCLEANUP_HANDLE:
                LOGD("Running auto-cleanup for REPY_Handle at 0x%p.", this_link->handle);
                PythonNative_Release(this_link->handle);
                break;
            case REPY_AUTOCLEANUP_RECOMP_FREE:
                LOGD("Running auto-cleanup for pointer (recomp_alloc/free) 0x%p.", this_link->pointer);
                recomp_free(this_link->pointer);
                break;
            case REPY_AUTOCLEANUP_ITERATORHELPER:
                LOGD("Running auto-cleanup for REPY_IteratorHelper at 0x%p.", this_link->pointer);
                REPY_IteratorHelperInternal_Destroy((REPY_IteratorHelperInternal*)this_link->pointer);
                break;
            case REPY_AUTOCLEANUP_IFSTMTHELPER:
                LOGD("Running auto-cleanup for REPY_IfStmtHelper at 0x%p.", this_link->pointer);
                REPY_IfStmtHelperInternal_Destroy((REPY_IfStmtHelperInternal*) this_link->pointer);
                break;
            default:
                LOGW(
                    "Invalid auto-cleanup type %u (handle = 0x%08X, pointer = 0x%p). Not sure how this happened, likely a bug with REPY itself. " \
                    "Doing nothing. This will likely result in a memory leak.", this_link->cleanup_type, this_link->handle, this_link->pointer);
                break;
        }

        REPY_AutoCleanupHelperInternal_Link* next_link = this_link->next;
        recomp_free(this_link);
        this_link = next_link;
    }
    // Makes the chain reusable if we so wish.
    cleanup->chain_start = NULL;
}

void REPY_DeferredCleanupHelperInternal_Destroy(REPY_DeferredCleanupHelperInternal* cleanup, bool clean_now) {
    if (clean_now) {
        REPY_DeferredCleanupHelperInternal_CleanNow(cleanup);
    }

    recomp_free(cleanup);
}