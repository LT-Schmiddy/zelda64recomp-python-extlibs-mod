#ifndef __EXTLIB_FUNCTIONS__
#define __EXTLIB_FUNCTIONS__

#include "modding.h"
#include "global.h"
#include "repy_api.h"


// General
RECOMP_IMPORT(".", void PythonNative_Preinit_RegisterNrmInModuleSearchPath(const unsigned char* str));
RECOMP_IMPORT(".", int PythonNative_Init(u32 log_level, u32 log_to_file, const unsigned char* str));
RECOMP_IMPORT(".", void PythonNative_Release(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_MakeSUH(REPY_Handle py_object));
RECOMP_IMPORT(".", u32 PythonNative_IsValidHandle(REPY_Handle py_object));
RECOMP_IMPORT(".", u32 PythonNative_GetSUH(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_SetSUH(REPY_Handle py_object, u32 suh_status));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CopyHandle(REPY_Handle py_object));

// Modules:
RECOMP_IMPORT(".", void PythonNative_ConstructModuleFromCStr(const char* identifier, const char* code, u32 add_to_sys));
RECOMP_IMPORT(".", void PythonNative_ConstructModuleFromCStrN(const char* identifier, const char* code, u32 len, u32 add_to_sys));
RECOMP_IMPORT(".", REPY_Handle PythonNative_ImportModule(const char* identifier));

// Casting Primatives:
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateBool(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_CastBool(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateU8(u8 value));
RECOMP_IMPORT(".", u8 PythonNative_CastU8(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateS8(s8 value));
RECOMP_IMPORT(".", s8 PythonNative_CastS8(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateU16(u16 value));
RECOMP_IMPORT(".", u16 PythonNative_CastU16(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateS16(s16 value));
RECOMP_IMPORT(".", s16 PythonNative_CastS16(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateU32(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_CastU32(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateS32(s32 value));
RECOMP_IMPORT(".", s32 PythonNative_CastS32(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateF32(f32 value));
RECOMP_IMPORT(".", f32 PythonNative_CastF32(REPY_Handle py_object));

// 64-bit Primatives:
RECOMP_IMPORT(".", REPY_Handle PythonNative_U64Operation(u64* location, REPY_Handle handle));
RECOMP_IMPORT(".", REPY_Handle PythonNative_S64Operation(s64* location, REPY_Handle handle));
RECOMP_IMPORT(".", REPY_Handle PythonNative_F64Operation(f64* location, REPY_Handle handle));

// Casting Strings
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateStr(const char* string));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_CastStr_Prepare(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_CastStr_Copy(u32 len, char* dst));

RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateByteStr(const char* string));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateByteStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_CastByteStr_Prepare(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_CastByteStr_Copy(u32 len, char* dst));

// Memcpy
RECOMP_IMPORT(".", REPY_Handle PythonNative_MemcpyToBytes(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_MemcpyFromBytes(void* src, u32 len, u32 reverse, REPY_Handle py_obj));
RECOMP_IMPORT(".", REPY_Handle PythonNative_MemcpyToByteArray(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_MemcpyFromByteArray(void* src, u32 len, u32 reverse, REPY_Handle py_obj));

// Indexing and Slicing
RECOMP_IMPORT(".", u32 PythonNative_Len(REPY_Handle object));

// Object Attribute Operations:
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetAttr(REPY_Handle object, REPY_Handle key, REPY_Handle default_value));
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetAttrCStr(REPY_Handle object,  char* key, REPY_Handle default_value));
RECOMP_IMPORT(".", void PythonNative_SetAttr(REPY_Handle object, REPY_Handle key, REPY_Handle value));
RECOMP_IMPORT(".", void PythonNative_SetAttrCStr(REPY_Handle object,  char* key, REPY_Handle value));
RECOMP_IMPORT(".", u32 PythonNative_HasAttr(REPY_Handle object, REPY_Handle key));
RECOMP_IMPORT(".", u32 PythonNative_HasAttrCStr(REPY_Handle object,  char* key));
RECOMP_IMPORT(".", void PythonNative_DelAttr(REPY_Handle object, REPY_Handle key));
RECOMP_IMPORT(".", void PythonNative_DelAttrCStr(REPY_Handle object,  char* key));

// Iteration
RECOMP_IMPORT(".", REPY_Handle PythonNative_Iter(REPY_Handle object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Next(REPY_Handle iter, REPY_Handle default_obj, u32 process_stop_iteration));

// Tuple:
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateTuple(u32 size, va_list va));
RECOMP_IMPORT(".", REPY_Handle PythonNative_TupleGetIndexS32(REPY_Handle object, int index));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreatePairCStr(char* key, REPY_Handle value));

// Dict Operations:
RECOMP_IMPORT(".", REPY_Handle PythonNative_CreateDict(u32 size, va_list va));
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetDict(REPY_Handle dict, REPY_Handle key));
RECOMP_IMPORT(".", REPY_Handle PythonNative_DictGetCStr(REPY_Handle dict, char* key));
RECOMP_IMPORT(".", void PythonNative_DictSet(REPY_Handle dict, REPY_Handle key, REPY_Handle value));
RECOMP_IMPORT(".", void PythonNative_DictSetCStr(REPY_Handle dict, char* key, REPY_Handle value));
RECOMP_IMPORT(".", u32 PythonNative_DictHas(REPY_Handle dict, REPY_Handle key));
RECOMP_IMPORT(".", u32 PythonNative_DictHasCStr(REPY_Handle dict, char* key));
RECOMP_IMPORT(".", void PythonNative_DictDel(REPY_Handle dict, REPY_Handle key));
RECOMP_IMPORT(".", void PythonNative_DictDelCStr(REPY_Handle dict, char* key));


// Execution Operations:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Compile(REPY_Handle code, REPY_Handle itentifier, REPY_Handle mode));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CompileCStr(const char* code, const char* identifier, REPY_CodeMode mode));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CompileCStrN(const char* code, u32 len,  const char* identifier, REPY_CodeMode mode));
RECOMP_IMPORT(".", u32 PythonNative_Exec(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", u32 PythonNative_ExecCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", u32 PythonNative_ExecCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Eval(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_EvalCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_EvalCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_ConstructVL(REPY_Handle local_scope, u32 len, va_list va));
RECOMP_IMPORT(".", REPY_Handle PythonNative_EvalVL(REPY_Handle code, REPY_Handle global_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_EvalVLCStr(const char* code, REPY_Handle global_scope));
RECOMP_IMPORT(".", REPY_Handle PythonNative_EvalVLCStrN(const char* code, u32 len, REPY_Handle global_scope));

// Function Calling Operations:
RECOMP_IMPORT(".", u32 PythonNative_Call(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CallReturn(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", u32 PythonNative_CallAttr(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", u32 PythonNative_CallAttrCStr(REPY_Handle func, const char* name, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CallAttrReturn(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CallAttrCStrReturn(REPY_Handle func, const char* name, REPY_Handle args, REPY_Handle kwargs));

// Exception Handling:
RECOMP_IMPORT(".", u32 PythonNative_IsErrorSet());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorType());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorTrace());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorValue());
RECOMP_IMPORT(".", void PythonNative_ClearError());

RECOMP_IMPORT(".", REPY_Handle PythonNative_GetZipFileFromPathCStr(const char* filepath));

// Logging
RECOMP_IMPORT(".", void PythonNative_SetLogMetaData(u32 log_level, const char* func, u32 line_number, const char* file_name));
RECOMP_IMPORT(".", void PythonNative_CommitLogMessage(const char* message));

#endif