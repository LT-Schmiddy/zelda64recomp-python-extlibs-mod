#include "helper_auto_cleanup.h"

#include "recomputils.h"
#include "../extlib_functions.h"
#include "../mod_logging.h"

REPY_HelperAutoCleanupInternal* REPY_HelperAutoCleanupInternal_Create() {
    REPY_HelperAutoCleanupInternal* retVal = recomp_alloc(sizeof(REPY_HelperAutoCleanupInternal));
    retVal->chain_start = NULL;

    return retVal;
}

REPY_IteratorHelperInternal* REPY_HelperAutoCleanupInternal_AddIteratorHelper(REPY_HelperAutoCleanupInternal* cleanup, REPY_IteratorHelperInternal* iterator_helper) {
    // Create a new link and configure it:
    REPY_HelperAutoCleanupInternal_Link* new_link = recomp_alloc(sizeof(REPY_HelperAutoCleanupInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_ITERATORHELPER;
    new_link->helper = (void*) iterator_helper;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("REPY_IteratorHelper at %p queued for auto-cleanup.", iterator_helper);
    return iterator_helper;
}

REPY_IfStmtHelperInternal* REPY_HelperAutoCleanupInternal_AddIfStmtHelper(REPY_HelperAutoCleanupInternal* cleanup, REPY_IfStmtHelperInternal* if_stmt_helper) {
    // Create a new link and configure it:
    REPY_HelperAutoCleanupInternal_Link* new_link = recomp_alloc(sizeof(REPY_HelperAutoCleanupInternal_Link));
    new_link->cleanup_type = REPY_AUTOCLEANUP_IFSTMTHELPER;
    new_link->helper = (void*) if_stmt_helper;
    
    // make this the new start of the chain.
    new_link->next = cleanup->chain_start;
    cleanup->chain_start = new_link;

    LOG_DEBUG("REPY_IfStmtHelper at %p queued for auto-cleanup.", if_stmt_helper);
    return if_stmt_helper;
}

void REPY_HelperAutoCleanupInternal_CleanNow(REPY_HelperAutoCleanupInternal* cleanup) {
    // Traverse the chain, destroying each helper as we go:
    REPY_HelperAutoCleanupInternal_Link* this_link = cleanup->chain_start;

    while(this_link != NULL) {
        switch (this_link->cleanup_type) {
            case REPY_AUTOCLEANUP_ITERATORHELPER:
                LOGD("Running auto-cleanup for REPY_IteratorHelper at %p.", this_link->helper);
                REPY_IteratorHelperInternal_Destroy((REPY_IteratorHelperInternal*)this_link->helper );
                break;
            case REPY_AUTOCLEANUP_IFSTMTHELPER:
                LOGD("Running auto-cleanup for REPY_IfStmtHelper at %p.", this_link->helper);
                REPY_IfStmtHelperInternal_Destroy((REPY_IfStmtHelperInternal*) this_link->helper);
                break;
            default:
                LOGW("Invalid auto-cleanup type %u for pointer %p. Doing nothing. This may result in a memory leak.", this_link->cleanup_type, this_link->helper);
                break;
        }

        REPY_HelperAutoCleanupInternal_Link* next_link = this_link->next;
        recomp_free(this_link);
        next_link = this_link;
    }
}

void REPY_HelperAutoCleanupInternal_Destroy(REPY_HelperAutoCleanupInternal* cleanup, bool clean_now) {
    if (clean_now) {
        REPY_HelperAutoCleanupInternal_CleanNow(cleanup);
    }

    recomp_free(cleanup);
}