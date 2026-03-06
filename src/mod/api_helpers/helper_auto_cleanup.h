#ifndef __HELPER_AUTO_CLEANUP__
#define __HELPER_AUTO_CLEANUP__

#include "repy_api.h"
#include "global.h"
#include "if_stmt_helper.h"
#include "iterator_helper.h"

typedef enum REPY_HelperAutoCleanupInternal_Type {
    REPY_AUTOCLEANUP_UNUSED = 0, // Reserved if I ever want to add generic deallocation via `recomp_free`
    REPY_AUTOCLEANUP_ITERATORHELPER = 1,
    REPY_AUTOCLEANUP_IFSTMTHELPER = 2
} REPY_HelperAutoCleanupInternal_Type;

typedef struct REPY_HelperAutoCleanupInternal_Link {
    REPY_HelperAutoCleanupInternal_Type cleanup_type;
    void* helper;
    REPY_HelperAutoCleanupInternal* next;
} REPY_HelperAutoCleanupInternal_Link;

typedef struct REPY_HelperAutoCleanupInternal {
    REPY_HelperAutoCleanupInternal_Link* chain_start;
} REPY_HelperAutoCleanupInternal;

REPY_HelperAutoCleanupInternal* REPY_HelperAutoCleanupInternal_Create();
REPY_IteratorHelperInternal* REPY_HelperAutoCleanupInternal_AddIteratorHelper(REPY_HelperAutoCleanupInternal* cleanup, REPY_IteratorHelperInternal* iterator_helper);
REPY_IfStmtHelperInternal* REPY_HelperAutoCleanupInternal_AddIfStmtHelper(REPY_HelperAutoCleanupInternal* cleanup, REPY_IfStmtHelperInternal* if_stmt_helper);
void REPY_HelperAutoCleanupInternal_CleanNow(REPY_HelperAutoCleanupInternal* cleanup);
void REPY_HelperAutoCleanupInternal_Destroy(REPY_HelperAutoCleanupInternal* cleanup, bool clean_now);

#endif