#ifndef __EXTLIB_FUNCTIONS__
#define __EXTLIB_FUNCTIONS__

#include "modding.h"
#include "global.h"
#include "libc/stdarg.h"
#include "repy_api.h"


// General
RECOMP_IMPORT(".", int PythonNative_Init(u32 log_level, const unsigned char* str));
RECOMP_IMPORT(".", void PythonNative_Object_Release(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_MakeSUH(REPY_Handle py_object));
RECOMP_IMPORT(".", u32 PythonNative_Object_IsValidHandle(REPY_Handle py_object));
RECOMP_IMPORT(".", u32 PythonNative_Object_GetSUH(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_SetSUH(REPY_Handle py_object, u32 suh_status));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CopyHandle(REPY_Handle py_object));

// Modules:
RECOMP_IMPORT(".", void PythonNative_LoadModule(const char* identifier, const char* code));
RECOMP_IMPORT(".", void PythonNative_LoadModuleN(const char* identifier, const char* code, u32 len));
RECOMP_IMPORT(".", REPY_Handle PythonNative_ImportModule(const char* identifier));

// Casting Primatives:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateBool(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastBool(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateU8(u8 value));
RECOMP_IMPORT(".", u8 PythonNative_Object_CastU8(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateS8(s8 value));
RECOMP_IMPORT(".", s8 PythonNative_Object_CastS8(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateU16(u16 value));
RECOMP_IMPORT(".", u16 PythonNative_Object_CastU16(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateS16(s16 value));
RECOMP_IMPORT(".", s16 PythonNative_Object_CastS16(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateU32(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastU32(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateS32(s32 value));
RECOMP_IMPORT(".", s32 PythonNative_Object_CastS32(REPY_Handle py_object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateF32(f32 value));
RECOMP_IMPORT(".", f32 PythonNative_Object_CastF32(REPY_Handle py_object));

// 64-bit Primatives:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_U64Operation(u64* location, REPY_Handle handle));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_S64Operation(s64* location, REPY_Handle handle));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_F64Operation(f64* location, REPY_Handle handle));

// Casting Strings
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateStr(const char* string));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastStr_Prepare(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_CastStr_Copy(u32 len, char* dst));

RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateByteStr(const char* string));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_CreateByteStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastByteStr_Prepare(REPY_Handle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_CastByteStr_Copy(u32 len, char* dst));

// Memcpy
RECOMP_IMPORT(".", REPY_Handle PythonNative_Memcpy_ToBytes(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_Memcpy_FromBytes(void* src, u32 len, u32 reverse, REPY_Handle py_obj));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Memcpy_ToByteArray(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_Memcpy_FromByteArray(void* src, u32 len, u32 reverse, REPY_Handle py_obj));

// Indexing and Slicing
RECOMP_IMPORT(".", u32 PythonNative_Object_Len(REPY_Handle object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_GetIndex(REPY_Handle object, int index));

// Iteration
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_Iter(REPY_Handle object));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_Next(REPY_Handle iter, REPY_Handle default_obj, u32 process_stop_iteration));

// Tuple:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Tuple_Create(u32 size, va_list va));

// Dict Operations:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Dict_Create(u32 size, va_list va));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Dict_Get(REPY_Handle dict, REPY_Handle key));
RECOMP_IMPORT(".", void PythonNative_Dict_Set(REPY_Handle dict, REPY_Handle key, REPY_Handle value));
RECOMP_IMPORT(".", u32 PythonNative_Dict_Has(REPY_Handle dict, REPY_Handle key));
RECOMP_IMPORT(".", void PythonNative_Dict_Remove(REPY_Handle dict, REPY_Handle key));

// Object Attribute Operations:
RECOMP_IMPORT(".", REPY_Handle PythonNative_Object_GetAttr(REPY_Handle object, REPY_Handle key, REPY_Handle default_value));
RECOMP_IMPORT(".", void PythonNative_Object_SetAttr(REPY_Handle object, REPY_Handle key, REPY_Handle value));
RECOMP_IMPORT(".", u32 PythonNative_Object_HasAttr(REPY_Handle object, REPY_Handle key));
RECOMP_IMPORT(".", void PythonNative_Object_DelAttr(REPY_Handle object, REPY_Handle key));

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
RECOMP_IMPORT(".", u32 PythonNative_Call(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", REPY_Handle PythonNative_Call_Return(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", u32 PythonNative_CallAttr(REPY_Handle func, const char* name, REPY_Handle args, REPY_Handle kwargs));
RECOMP_IMPORT(".", REPY_Handle PythonNative_CallAttr_Return(REPY_Handle func, const char* name, REPY_Handle args, REPY_Handle kwargs));

// Exception Handling:
RECOMP_IMPORT(".", u32 PythonNative_IsErrorSet());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorType());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorTrace());
RECOMP_IMPORT(".", REPY_Handle PythonNative_GetErrorValue());
RECOMP_IMPORT(".", void PythonNative_ClearError());

#endif