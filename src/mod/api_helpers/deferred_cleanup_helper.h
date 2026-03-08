#ifndef __HELPER_AUTO_CLEANUP__
#define __HELPER_AUTO_CLEANUP__

#include "repy_api.h"
#include "global.h"
#include "if_stmt_helper.h"
#include "iterator_helper.h"


typedef enum REPY_AutoCleanupHelperInternal_Type {
    REPY_AUTOCLEANUP_HANDLE = 0, 
    REPY_AUTOCLEANUP_RECOMP_FREE = 1, 
    REPY_AUTOCLEANUP_ITERATORHELPER = 2,
    REPY_AUTOCLEANUP_IFSTMTHELPER = 3
} REPY_AutoCleanupHelperInternal_Type;

typedef struct REPY_AutoCleanupHelperInternal_Link {
    REPY_AutoCleanupHelperInternal_Type cleanup_type;
    REPY_Handle handle;
    void* pointer;
    struct REPY_AutoCleanupHelperInternal_Link* next;
} REPY_AutoCleanupHelperInternal_Link;

typedef struct REPY_DeferredCleanupHelperInternal {
    REPY_AutoCleanupHelperInternal_Link* chain_start;
} REPY_DeferredCleanupHelperInternal;

REPY_DeferredCleanupHelperInternal* REPY_DeferredCleanupHelperInternal_Create();
REPY_Handle REPY_DeferredCleanupHelperInternal_AddHandle(REPY_DeferredCleanupHelperInternal* cleanup, REPY_Handle handle);
void* REPY_DeferredCleanupHelperInternal_AddRecompFree(REPY_DeferredCleanupHelperInternal* cleanup, void* pointer);
REPY_IteratorHelperInternal* REPY_DeferredCleanupHelperInternal_AddIteratorHelper(REPY_DeferredCleanupHelperInternal* cleanup, REPY_IteratorHelperInternal* iterator_helper);
REPY_IfStmtHelperInternal* REPY_DeferredCleanupHelperInternal_AddIfStmtHelper(REPY_DeferredCleanupHelperInternal* cleanup, REPY_IfStmtHelperInternal* if_stmt_helper);
void REPY_DeferredCleanupHelperInternal_CleanNow(REPY_DeferredCleanupHelperInternal* cleanup);
void REPY_DeferredCleanupHelperInternal_Destroy(REPY_DeferredCleanupHelperInternal* cleanup, bool clean_now);

#endif