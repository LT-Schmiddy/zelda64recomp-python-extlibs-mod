#ifndef __EXTLIB_FUNCTIONS__
#define __EXTLIB_FUNCTIONS__

#include "modding.h"
#include "global.h"

typedef int PythonBytecodeHandle;
typedef int PythonScopeHandle;
typedef int PythonObjectHandle;

typedef enum PythonCodeType {
    PY_CODE_EXEC = 0,
    PY_CODE_EVAL = 1,
    PY_CODE_SINGLE = 2
} PythonCodeType;


RECOMP_IMPORT(".", int PythonNative_Init(const unsigned char* str));

RECOMP_IMPORT(".", PythonScopeHandle PythonNative_CreateScope());
RECOMP_IMPORT(".", void PythonNative_ReleaseScope(PythonScopeHandle scope));

RECOMP_IMPORT(".", u32 PythonNative_Scope_GetBool(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetBool(u32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", u32 PythonNative_Scope_GetU32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetU32(u32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", s32 PythonNative_Scope_GetS32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetS32(s32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", f32 PythonNative_Scope_GetF32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetF32(f32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetString(char* value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetStringN(char* value, u32 len, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetString_Prepare(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetString_Copy(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetBytes(char* value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetBytesN(char* value, u32 len, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetBytes_Prepare(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetBytes_Copy(PythonScopeHandle scope, const char* name));

RECOMP_IMPORT(".", PythonBytecodeHandle PythonNative_CompileBytecode(const char* str, const char* identifier, PythonCodeType type));
RECOMP_IMPORT(".", PythonBytecodeHandle PythonNative_CompileBytecodeN(const char* str, u32 len, const char* identifier, PythonCodeType type));
RECOMP_IMPORT(".", void PythonNative_ReleaseBytecode(PythonBytecodeHandle bytecode));
RECOMP_IMPORT(".", int PythonNative_Execute(PythonBytecodeHandle handle, PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_ExecuteString(const char* str, PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_ExecuteStringN(const char* str, PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_LoadModule(const char* module_name, const char* module_code));
RECOMP_IMPORT(".", int PythonNative_LoadModuleN(const char* module_name, const char* module_code, unsigned int len));




#endif