#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"

#include "extlib_functions.h"

RECOMP_DECLARE_EVENT(REPY_OnMakeGlobalCaches(int success));
RECOMP_DECLARE_EVENT(REPY_OnInit(int success));

RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int py_init = PythonNative_Init(recomp_get_config_u32("log_level"), mod_folder);
    recomp_free((void*)mod_folder);

    if (py_init == 1) {
        recomp_printf("Python interpreter initialized successfully.\n");
    } else {
        recomp_printf("There was an error initializing the Python interpreter.\n");
    }

    REPY_OnMakeGlobalCaches(py_init);
    REPY_OnInit(py_init);
}

// === EXPORTED FUNCTIONS === 
// General:
RECOMP_EXPORT void REPY_Release(PyObjectHandle py_object) {
    PythonNative_Object_Release(py_object);
}

RECOMP_EXPORT PyObjectHandle REPY_MakeSUH(PyObjectHandle py_object) {
    return PythonNative_Object_MakeSUH(py_object);
}

RECOMP_EXPORT bool REPY_IsValidHandle(PyObjectHandle py_object) {
    return PythonNative_Object_IsValidHandle(py_object);
}

RECOMP_EXPORT bool REPY_GetSUH(PyObjectHandle py_object) {
    return PythonNative_Object_GetSUH(py_object);
}

RECOMP_EXPORT void REPY_SetSUH(PyObjectHandle py_object, bool value) {
    PythonNative_Object_SetSUH(py_object, value);
}

RECOMP_EXPORT PyObjectHandle REPY_CopyHandle(PyObjectHandle py_object) {
    return PythonNative_Object_CopyHandle(py_object);
}

// Modules:
RECOMP_EXPORT void REPY_LoadModule(const char* identifier, const char* code) {
    PythonNative_LoadModule(identifier, code);
}

RECOMP_EXPORT void REPY_LoadModuleN(const char* identifier, const char* code, u32 len) {
    PythonNative_LoadModuleN(identifier, code, len);
}

RECOMP_EXPORT PyObjectHandle REPY_ImportModule(const char* identifier) {
    return PythonNative_ImportModule(identifier);
}

// Primatives:
RECOMP_EXPORT PyObjectHandle REPY_CreateBool(bool value) {
    return PythonNative_Object_CreateBool(value);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateBool_SUH(bool value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateBool(value));
}

RECOMP_EXPORT bool REPY_CastBool(PyObjectHandle object) {
    return PythonNative_Object_CastBool(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateU32(u32 value) {
    return PythonNative_Object_CreateU32(value);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateU32_SUH(u32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateU32(value));
}

RECOMP_EXPORT u32 REPY_CastU32(PyObjectHandle object) {
    return PythonNative_Object_CastU32(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateS32(s32 value) {
    return PythonNative_Object_CreateS32(value);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateS32_SUH(s32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateS32(value));
}

RECOMP_EXPORT s32 REPY_CastS32(PyObjectHandle object) {
    return PythonNative_Object_CastS32(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateF32(f32 value) {
    return PythonNative_Object_CreateF32(value);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateF32_SUH(f32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateF32(value));
}

RECOMP_EXPORT f32 REPY_CastF32(PyObjectHandle object) {
    return PythonNative_Object_CastF32(object);
}

// 64-bit Primatives:
RECOMP_EXPORT PyObjectHandle REPY_CreateU64(u64 value) {
    return PythonNative_Object_U64Operation(&value, 0);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateU64_SUH(u64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_U64Operation(&value, 0));
}

RECOMP_EXPORT u64 REPY_CastU64(PyObjectHandle object) {
    u64 retVal;
    PythonNative_Object_U64Operation(&retVal, object);
    return retVal;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateS64(s64 value) {
    return PythonNative_Object_S64Operation(&value, 0);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateS64_SUH(s64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_S64Operation(&value, 0));
}

RECOMP_EXPORT s64 REPY_CastS64(PyObjectHandle object) {
    s64 retVal;
    PythonNative_Object_S64Operation(&retVal, object);
    return retVal;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateF64(f64 value) {
    return PythonNative_Object_F64Operation(&value, 0);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateF64_SUH(f64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_F64Operation(&value, 0));
}

RECOMP_EXPORT f64 REPY_CastF64(PyObjectHandle object) {
    f64 retVal;
    PythonNative_Object_F64Operation(&retVal, object);
    return retVal;
}

// Strings:
RECOMP_EXPORT PyObjectHandle REPY_CreateStr(const char* string) {
    return PythonNative_Object_CreateStr(string);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateStr_SUH(const char* string) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateStr(string));
}

RECOMP_EXPORT PyObjectHandle REPY_CreateStrN(const char* string, u32 len) {
    return PythonNative_Object_CreateStrN(string, len);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateStrN_SUH(const char* string, u32 len) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateStrN(string, len));
}

RECOMP_EXPORT char* REPY_CastStr(PyObjectHandle object) {
    u32 len = PythonNative_Object_CastStr_Prepare(object);
    char* retVal = recomp_alloc(len);
    PythonNative_Object_CastStr_Copy(len, retVal);
    return retVal;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateByteStr(const char* string) {
    return PythonNative_Object_CreateByteStr(string);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateByteStr_SUH(const char* string) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateByteStr(string));
}

RECOMP_EXPORT PyObjectHandle REPY_CreateByteStrN(const char* string, u32 len) {
    return PythonNative_Object_CreateByteStrN(string, len);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateByteStrN_SUH(const char* string, u32 len) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateByteStrN(string, len));
}

RECOMP_EXPORT char* REPY_CastByteStr(PyObjectHandle object) {
    u32 len = PythonNative_Object_CastByteStr_Prepare(object);
    char* retVal = recomp_alloc(len);
    PythonNative_Object_CastByteStr_Copy(len, retVal);
    return retVal;
}
// Tuple Operations:
RECOMP_EXPORT PyObjectHandle REPY_CreateTuple(u32 size, ...) {
    va_list va;
    va_start(va, size);
    PyObjectHandle handle = PythonNative_Tuple_Create(size, va);
    va_end(va);

    return handle;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateTuple_SUH(u32 size, ...) {
    va_list va;
    va_start(va, size);
    PyObjectHandle handle = PythonNative_Tuple_Create(size, va);
    va_end(va);

    return PythonNative_Object_MakeSUH(handle);
}

RECOMP_EXPORT PyObjectHandle REPY_CreatePair(PyObjectHandle key, PyObjectHandle value) {
    return REPY_CreateTuple(2, key, value);
}

RECOMP_EXPORT PyObjectHandle REPY_CreatePair_SUH(PyObjectHandle key, PyObjectHandle value) {
    return REPY_CreateTuple_SUH(2, key, value);
}

RECOMP_EXPORT PyObjectHandle REPY_TupleGetMember(PyObjectHandle tuple, int index) {
    return PythonNative_Tuple_GetMember(tuple, index);
}

// Dict Operations:
RECOMP_EXPORT PyObjectHandle REPY_CreateEmptyDict() {
    return PythonNative_Dict_Create(0, NULL);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateEmptyDict_SUH() {
    return PythonNative_Object_MakeSUH(PythonNative_Dict_Create(0, NULL));
}

RECOMP_EXPORT PyObjectHandle REPY_CreateDict(u32 size, ...) {
    va_list va;
    va_start(va, size);
    PyObjectHandle handle = PythonNative_Dict_Create(size, va);
    va_end(va);

    return handle;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateDict_SUH(u32 size, ...) {
    va_list va;
    va_start(va, size);
    PyObjectHandle handle = PythonNative_Dict_Create(size, va);
    va_end(va);

    return PythonNative_Object_MakeSUH(handle);
}

RECOMP_EXPORT PyObjectHandle REPY_DictGet(PyObjectHandle dict, PyObjectHandle key) {
    return PythonNative_Dict_Get(dict, key);
}

RECOMP_EXPORT void REPY_DictSet(PyObjectHandle dict, PyObjectHandle key, PyObjectHandle value) {
    PythonNative_Dict_Set(dict, key, value);
}

RECOMP_EXPORT bool REPY_DictHas(PyObjectHandle dict, PyObjectHandle key) {
    return PythonNative_Dict_Has(dict, key);
}

RECOMP_EXPORT void REPY_DictDel(PyObjectHandle dict, PyObjectHandle key) {
    PythonNative_Dict_Remove(dict, key);
}

// Execution:
RECOMP_EXPORT PyObjectHandle REPY_Compile(PyObjectHandle code, PyObjectHandle identifier, PyObjectHandle mode) {
    return PythonNative_Compile(code, identifier, mode);
}

RECOMP_EXPORT PyObjectHandle REPY_CompileCStr(const char* code, const char* identifier, PythonCodeMode mode) {
    return PythonNative_CompileCStr(code, identifier, mode);
}

RECOMP_EXPORT PyObjectHandle REPY_CompileCStrN(const char* code, u32 len, const char* identifier, PythonCodeMode mode) {
    return PythonNative_CompileCStrN(code, len, identifier, mode);
}

RECOMP_EXPORT bool REPY_Exec(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_Exec(code, global_scope, local_scope);
}

RECOMP_EXPORT bool REPY_ExecCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_ExecCStr(code, global_scope, local_scope);
}

RECOMP_EXPORT bool REPY_ExecCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_ExecCStrN(code, len, global_scope, local_scope);
}

RECOMP_EXPORT PyObjectHandle REPY_Eval(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_Eval(code, global_scope, local_scope);
}

RECOMP_EXPORT PyObjectHandle REPY_EvalCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_EvalCStr(code, global_scope, local_scope);
}

RECOMP_EXPORT PyObjectHandle REPY_EvalCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope) {
    return PythonNative_EvalCStrN(code, len, global_scope, local_scope);
}

// Python Functions
RECOMP_EXPORT bool REPY_Call(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs) {
    return PythonNative_Call(func, args, kwargs);
}

RECOMP_EXPORT PyObjectHandle REPY_CallReturn(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs) {
    return PythonNative_Call_Return(func, args, kwargs);
}

RECOMP_EXPORT bool REPY_CallAttr(PyObjectHandle func, char* name, PyObjectHandle args, PyObjectHandle kwargs) {
    return PythonNative_CallAttr(func, name, args, kwargs);
}

RECOMP_EXPORT PyObjectHandle REPY_CallAttrReturn(PyObjectHandle func, char* name, PyObjectHandle args, PyObjectHandle kwargs) {
    return PythonNative_CallAttr_Return(func, name, args, kwargs);
}

// Exception Handling:
RECOMP_EXPORT bool REPY_IsErrorSet() {
    return PythonNative_IsErrorSet();
}

RECOMP_EXPORT PyObjectHandle REPY_GetErrorType() {
    return PythonNative_GetErrorType();
}

RECOMP_EXPORT PyObjectHandle REPY_GetErrorTrace() {
    return PythonNative_GetErrorTrace();
}

RECOMP_EXPORT PyObjectHandle REPY_GetErrorValue() {
    return PythonNative_GetErrorValue();
}

RECOMP_EXPORT void REPY_ClearError() {
    PythonNative_ClearError();
}




