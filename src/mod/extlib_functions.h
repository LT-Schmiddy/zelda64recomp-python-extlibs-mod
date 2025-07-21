#ifndef __EXTLIB_FUNCTIONS__
#define __EXTLIB_FUNCTIONS__

#include "modding.h"
#include "global.h"
#include "libc/stdarg.h"
#include "repy_api.h"


// General
RECOMP_IMPORT(".", int PythonNative_Init(u32 log_level, const unsigned char* str));
RECOMP_IMPORT(".", void PythonNative_Object_Release(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_MakeSUH(PyObjectHandle py_object));
RECOMP_IMPORT(".", u32 PythonNative_Object_IsValidHandle(PyObjectHandle py_object));
RECOMP_IMPORT(".", u32 PythonNative_Object_GetSUH(PyObjectHandle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_SetSUH(PyObjectHandle py_object, u32 suh_status));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CopyHandle(PyObjectHandle py_object));

// Modules:
RECOMP_IMPORT(".", void PythonNative_LoadModule(const char* identifier, const char* code));
RECOMP_IMPORT(".", void PythonNative_LoadModuleN(const char* identifier, const char* code, u32 len));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_ImportModule(const char* identifier));

// Casting Primatives:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateBool(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastBool(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateU8(u8 value));
RECOMP_IMPORT(".", u8 PythonNative_Object_CastU8(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateS8(s8 value));
RECOMP_IMPORT(".", s8 PythonNative_Object_CastS8(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateU16(u16 value));
RECOMP_IMPORT(".", u16 PythonNative_Object_CastU16(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateS16(s16 value));
RECOMP_IMPORT(".", s16 PythonNative_Object_CastS16(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateU32(u32 value));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastU32(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateS32(s32 value));
RECOMP_IMPORT(".", s32 PythonNative_Object_CastS32(PyObjectHandle py_object));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateF32(f32 value));
RECOMP_IMPORT(".", f32 PythonNative_Object_CastF32(PyObjectHandle py_object));

// 64-bit Primatives:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_U64Operation(u64* location, PyObjectHandle handle));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_S64Operation(s64* location, PyObjectHandle handle));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_F64Operation(f64* location, PyObjectHandle handle));

// Casting Strings
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateStr(const char* string));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastStr_Prepare(PyObjectHandle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_CastStr_Copy(u32 len, char* dst));

RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateByteStr(const char* string));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_CreateByteStrN(const char* string, u32 len));
RECOMP_IMPORT(".", u32 PythonNative_Object_CastByteStr_Prepare(PyObjectHandle py_object));
RECOMP_IMPORT(".", void PythonNative_Object_CastByteStr_Copy(u32 len, char* dst));

// Memcpy
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Memcpy_ToBytes(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_Memcpy_FromBytes(void* src, u32 len, u32 reverse, PyObjectHandle py_obj));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Memcpy_ToByteArray(void* src, u32 len, u32 reverse));
RECOMP_IMPORT(".", u32 PythonNative_Memcpy_FromByteArray(void* src, u32 len, u32 reverse, PyObjectHandle py_obj));

// Tuple:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Tuple_Create(u32 size, va_list va));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Tuple_GetMember(PyObjectHandle tuple, int index));

// Dict Operations:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Dict_Create(u32 size, va_list va));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Dict_Get(PyObjectHandle dict, PyObjectHandle key));
RECOMP_IMPORT(".", void PythonNative_Dict_Set(PyObjectHandle dict, PyObjectHandle key, PyObjectHandle value));
RECOMP_IMPORT(".", u32 PythonNative_Dict_Has(PyObjectHandle dict, PyObjectHandle key));
RECOMP_IMPORT(".", void PythonNative_Dict_Remove(PyObjectHandle dict, PyObjectHandle key));

// Object Attribute Operations:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Object_GetAttr(PyObjectHandle object, PyObjectHandle key, PyObjectHandle default_value));
RECOMP_IMPORT(".", void PythonNative_Object_SetAttr(PyObjectHandle object, PyObjectHandle key, PyObjectHandle value));
RECOMP_IMPORT(".", u32 PythonNative_Object_HasAttr(PyObjectHandle object, PyObjectHandle key));
RECOMP_IMPORT(".", void PythonNative_Object_DelAttr(PyObjectHandle object, PyObjectHandle key));

// Execution Operations:
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Compile(PyObjectHandle code, PyObjectHandle itentifier, PyObjectHandle mode));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_CompileCStr(const char* code, const char* identifier, PythonCodeMode mode));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_CompileCStrN(const char* code, u32 len,  const char* identifier, PythonCodeMode mode));
RECOMP_IMPORT(".", u32 PythonNative_Exec(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", u32 PythonNative_ExecCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", u32 PythonNative_ExecCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Eval(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_EvalCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_EvalCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope));
RECOMP_IMPORT(".", u32 PythonNative_Call(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_Call_Return(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs));
RECOMP_IMPORT(".", u32 PythonNative_CallAttr(PyObjectHandle func, const char* name, PyObjectHandle args, PyObjectHandle kwargs));
RECOMP_IMPORT(".", PyObjectHandle PythonNative_CallAttr_Return(PyObjectHandle func, const char* name, PyObjectHandle args, PyObjectHandle kwargs));

// Exception Handling:
RECOMP_IMPORT(".", u32 PythonNative_IsErrorSet());
RECOMP_IMPORT(".", PyObjectHandle PythonNative_GetErrorType());
RECOMP_IMPORT(".", PyObjectHandle PythonNative_GetErrorTrace());
RECOMP_IMPORT(".", PyObjectHandle PythonNative_GetErrorValue());
RECOMP_IMPORT(".", void PythonNative_ClearError());

#endif