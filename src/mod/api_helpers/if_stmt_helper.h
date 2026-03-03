#ifndef __IF_STMT_HELPER__
#define __IF_STMT_HELPER__

// Keeps my IDE happy:
#ifndef REPY_INTERNALS_EXPOSED
#define REPY_INTERNALS_EXPOSED
#endif

#include "repy_api.h"

REPY_IfStmtChainInternal* IfStmtChainInternal_Create(char* expr_string, char* filename, char* function_name, REPY_u32 line_number, char* identifier);
void IfStmtChainInternal_Destroy(REPY_IfStmtChainInternal* chain);

// Helper stuff:
void IfStmtHelperInternal_Reset(REPY_IfStmtHelperInternal* helper, REPY_IfStmtChainInternal** root);
REPY_IfStmtHelperInternal* IfStmtHelperInternal_Create(REPY_IfStmtChainInternal** chain_root);
void IfStmtHelperInternal_Destroy(REPY_IfStmtHelperInternal* helper);
REPY_bool IfStmtHelperInternal_Step(REPY_IfStmtHelperInternal* helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, REPY_u32 line_number, char* identifier);

#endif