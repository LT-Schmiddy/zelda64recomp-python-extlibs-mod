#ifndef __IF_STMT_HELPER__
#define __IF_STMT_HELPER__

#include "repy_api.h"
#include "./if_stmt_chain.h"

typedef struct REPY_IfStmtHelperInternal {
    REPY_u32 index; ///< The number of links down the chain we've gone.
    REPY_IfStmtChainInternal** root; ///< The start of the chain. A double pointer is used so that, the the chain doesn't exist yet, it can be initialized on the first call of `REPY_IfStmtHelper_Step`.
    REPY_IfStmtChainInternal* curr; ///< The most recently evaluated link in the chain.
    REPY_bool _first_step; ///< ///< Internal flag used to determine if the helper has been stepped for the first time.
} REPY_IfStmtHelperInternal;

void IfStmtHelperInternal_Reset(REPY_IfStmtHelperInternal* helper, REPY_IfStmtChainInternal** root);
REPY_IfStmtHelperInternal* IfStmtHelperInternal_Create(REPY_IfStmtChainInternal** chain_root);
void IfStmtHelperInternal_Destroy(REPY_IfStmtHelperInternal* helper);
REPY_bool IfStmtHelperInternal_Step(REPY_IfStmtHelperInternal* helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, REPY_u32 line_number, char* identifier);

#endif