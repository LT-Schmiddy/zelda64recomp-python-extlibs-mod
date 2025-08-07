#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"

#include "extlib_functions.h"
#include "mod_logging.h"

RECOMP_DECLARE_EVENT(REPY_OnLoadModules(int success));
RECOMP_DECLARE_EVENT(REPY_OnMakeGlobalCaches(int success));
RECOMP_DECLARE_EVENT(REPY_OnInit(int success));

RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* nrm_file_path = recomp_get_mod_file_path();
    PythonNative_Preinit_RegisterNrmInModuleSearchPath(nrm_file_path);
    recomp_free((void*)nrm_file_path);

    
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int py_init = PythonNative_Init(recomp_get_config_u32("log_level"), mod_folder);
    recomp_free((void*)mod_folder);

    if (py_init == 1) {
        LOGI("Python interpreter initialized successfully.\n");
    } else {
        LOGF("There was an error initializing the Python interpreter.\n");
    }

    REPY_OnLoadModules(py_init);
    REPY_OnMakeGlobalCaches(py_init);
    REPY_OnInit(py_init);
}

// === EXPORTED FUNCTIONS === 
// General:
RECOMP_EXPORT void REPY_Release(REPY_Handle py_object) {
    PythonNative_Object_Release(py_object);
}

RECOMP_EXPORT REPY_Handle REPY_MakeSUH(REPY_Handle py_object) {
    return PythonNative_Object_MakeSUH(py_object);
}

RECOMP_EXPORT bool REPY_IsValidHandle(REPY_Handle py_object) {
    return PythonNative_Object_IsValidHandle(py_object);
}

RECOMP_EXPORT bool REPY_GetSUH(REPY_Handle py_object) {
    return PythonNative_Object_GetSUH(py_object);
}

RECOMP_EXPORT void REPY_SetSUH(REPY_Handle py_object, bool value) {
    PythonNative_Object_SetSUH(py_object, value);
}

RECOMP_EXPORT REPY_Handle REPY_CopyHandle(REPY_Handle py_object) {
    return PythonNative_Object_CopyHandle(py_object);
}

// Modules:
RECOMP_EXPORT void REPY_LoadModule(const char* identifier, const char* code) {
    PythonNative_LoadModule(identifier, code);
}

RECOMP_EXPORT void REPY_LoadModuleN(const char* identifier, const char* code, u32 len) {
    PythonNative_LoadModuleN(identifier, code, len);
}

RECOMP_EXPORT REPY_Handle REPY_ImportModule(const char* identifier) {
    return PythonNative_ImportModule(identifier);
}

// Primatives:
RECOMP_EXPORT REPY_Handle REPY_CreateBool(bool value) {
    return PythonNative_Object_CreateBool(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateBool_SUH(bool value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateBool(value));
}

RECOMP_EXPORT bool REPY_CastBool(REPY_Handle object) {
    return PythonNative_Object_CastBool(object);
}


RECOMP_EXPORT REPY_Handle REPY_CreateU8(u8 value) {
    return PythonNative_Object_CreateU8(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU8_SUH(u8 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateU8(value));
}

RECOMP_EXPORT u8 REPY_CastU8(REPY_Handle object) {
    return PythonNative_Object_CastU8(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS8(s8 value) {
    return PythonNative_Object_CreateS8(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS8_SUH(s8 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateS8(value));
}

RECOMP_EXPORT s8 REPY_CastS8(REPY_Handle object) {
    return PythonNative_Object_CastS8(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU16(u16 value) {
    return PythonNative_Object_CreateU16(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU16_SUH(u16 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateU16(value));
}

RECOMP_EXPORT u16 REPY_CastU16(REPY_Handle object) {
    return PythonNative_Object_CastU16(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS16(s16 value) {
    return PythonNative_Object_CreateS16(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS16_SUH(s16 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateS16(value));
}

RECOMP_EXPORT s16 REPY_CastS16(REPY_Handle object) {
    return PythonNative_Object_CastS16(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU32(u32 value) {
    return PythonNative_Object_CreateU32(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU32_SUH(u32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateU32(value));
}

RECOMP_EXPORT u32 REPY_CastU32(REPY_Handle object) {
    return PythonNative_Object_CastU32(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS32(s32 value) {
    return PythonNative_Object_CreateS32(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS32_SUH(s32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateS32(value));
}

RECOMP_EXPORT s32 REPY_CastS32(REPY_Handle object) {
    return PythonNative_Object_CastS32(object);
}

RECOMP_EXPORT REPY_Handle REPY_CreateF32(f32 value) {
    return PythonNative_Object_CreateF32(value);
}

RECOMP_EXPORT REPY_Handle REPY_CreateF32_SUH(f32 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateF32(value));
}

RECOMP_EXPORT f32 REPY_CastF32(REPY_Handle object) {
    return PythonNative_Object_CastF32(object);
}

// 64-bit Primatives:
RECOMP_EXPORT REPY_Handle REPY_CreateU64(u64 value) {
    return PythonNative_Object_U64Operation(&value, 0);
}

RECOMP_EXPORT REPY_Handle REPY_CreateU64_SUH(u64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_U64Operation(&value, 0));
}

RECOMP_EXPORT u64 REPY_CastU64(REPY_Handle object) {
    u64 retVal;
    PythonNative_Object_U64Operation(&retVal, object);
    return retVal;
}

RECOMP_EXPORT REPY_Handle REPY_CreateS64(s64 value) {
    return PythonNative_Object_S64Operation(&value, 0);
}

RECOMP_EXPORT REPY_Handle REPY_CreateS64_SUH(s64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_S64Operation(&value, 0));
}

RECOMP_EXPORT s64 REPY_CastS64(REPY_Handle object) {
    s64 retVal;
    PythonNative_Object_S64Operation(&retVal, object);
    return retVal;
}

RECOMP_EXPORT REPY_Handle REPY_CreateF64(f64 value) {
    return PythonNative_Object_F64Operation(&value, 0);
}

RECOMP_EXPORT REPY_Handle REPY_CreateF64_SUH(f64 value) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_F64Operation(&value, 0));
}

RECOMP_EXPORT f64 REPY_CastF64(REPY_Handle object) {
    f64 retVal;
    PythonNative_Object_F64Operation(&retVal, object);
    return retVal;
}

// Strings:
RECOMP_EXPORT REPY_Handle REPY_CreateStr(const char* string) {
    return PythonNative_Object_CreateStr(string);
}

RECOMP_EXPORT REPY_Handle REPY_CreateStr_SUH(const char* string) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateStr(string));
}

RECOMP_EXPORT REPY_Handle REPY_CreateStrN(const char* string, u32 len) {
    return PythonNative_Object_CreateStrN(string, len);
}

RECOMP_EXPORT REPY_Handle REPY_CreateStrN_SUH(const char* string, u32 len) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateStrN(string, len));
}

RECOMP_EXPORT char* REPY_CastStr(REPY_Handle object) {
    u32 len = PythonNative_Object_CastStr_Prepare(object);
    char* retVal = recomp_alloc(len + 1);
    PythonNative_Object_CastStr_Copy(len, retVal);
    retVal[len] = 0; // Setting Null terminator
    return retVal;
}

RECOMP_EXPORT REPY_Handle REPY_CreateByteStr(const char* string) {
    return PythonNative_Object_CreateByteStr(string);
}

RECOMP_EXPORT REPY_Handle REPY_CreateByteStr_SUH(const char* string) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateByteStr(string));
}

RECOMP_EXPORT REPY_Handle REPY_CreateByteStrN(const char* string, u32 len) {
    return PythonNative_Object_CreateByteStrN(string, len);
}

RECOMP_EXPORT REPY_Handle REPY_CreateByteStrN_SUH(const char* string, u32 len) {
    return PythonNative_Object_MakeSUH(PythonNative_Object_CreateByteStrN(string, len));
}

RECOMP_EXPORT char* REPY_CastByteStr(REPY_Handle object) {
    u32 len = PythonNative_Object_CastByteStr_Prepare(object);
    char* retVal = recomp_alloc(len + 1);;
    PythonNative_Object_CastByteStr_Copy(len, retVal);
    retVal[len] = 0; // Setting Null terminator
    return retVal;
}

RECOMP_EXPORT REPY_Handle REPY_MemcpyToByteStr(void* src, u32 len, u32 reverse) {
    return PythonNative_Memcpy_ToBytes(src, len, reverse);
}

RECOMP_EXPORT u32 REPY_MemcpyFromByteStr(void* dst, u32 len, u32 reverse, REPY_Handle bytes_obj) {
    return PythonNative_Memcpy_FromBytes(dst, len, reverse, bytes_obj);
}

RECOMP_EXPORT void* REPY_AllocAndCopyByteStr(u32 reverse, REPY_Handle bytes_obj, u32* write_size) {
    u32 size = PythonNative_Object_Len(bytes_obj);
    void* retVal = recomp_alloc(size);
    u32 _write_size = PythonNative_Memcpy_FromBytes(retVal, size, reverse, bytes_obj);
    if (write_size != NULL) {
        *write_size = _write_size;
    }
    return retVal;
}

RECOMP_EXPORT REPY_Handle REPY_MemcpyToByteArray(void* src, u32 len, u32 reverse) {
    return PythonNative_Memcpy_ToByteArray(src, len, reverse);
}

RECOMP_EXPORT u32 REPY_MemcpyFromByteArray(void* dst, u32 len, u32 reverse, REPY_Handle bytes_obj) {
    return PythonNative_Memcpy_FromByteArray(dst, len, reverse, bytes_obj);
}

RECOMP_EXPORT void* REPY_AllocAndCopyByteArray(u32 reverse, REPY_Handle bytes_obj, u32* write_size) {
    u32 size = PythonNative_Object_Len(bytes_obj);
    void* retVal = recomp_alloc(size);
    u32 _write_size = PythonNative_Memcpy_FromByteArray(retVal, size, reverse, bytes_obj);
    if (write_size != NULL) {
        *write_size = _write_size;
    }
    return retVal;
}


// Indexing and Slicing:
RECOMP_EXPORT u32 REPY_Len(REPY_Handle object) {
    return PythonNative_Object_Len(object);
}

// Iteration
RECOMP_EXPORT REPY_Handle REPY_Iter(REPY_Handle object) {
    return PythonNative_Object_Iter(object);
}

RECOMP_EXPORT REPY_Handle REPY_Next(REPY_Handle iterator, REPY_Handle default_obj, u32 process_stop_iteration) {
    return PythonNative_Object_Next(iterator, default_obj, process_stop_iteration);
}

// Tuple Operations:
RECOMP_EXPORT REPY_Handle REPY_CreateTuple(u32 size, ...) {
    va_list va;
    va_start(va, size);
    REPY_Handle handle = PythonNative_Tuple_Create(size, va);
    va_end(va);

    return handle;
}

RECOMP_EXPORT REPY_Handle REPY_CreateTuple_SUH(u32 size, ...) {
    va_list va;
    va_start(va, size);
    REPY_Handle handle = PythonNative_Tuple_Create(size, va);
    va_end(va);

    return PythonNative_Object_MakeSUH(handle);
}

RECOMP_EXPORT REPY_Handle REPY_CreatePair(REPY_Handle key, REPY_Handle value) {
    return REPY_CreateTuple(2, key, value);
}

RECOMP_EXPORT REPY_Handle REPY_CreatePair_SUH(REPY_Handle key, REPY_Handle value) {
    return REPY_CreateTuple_SUH(2, key, value);
}

RECOMP_EXPORT REPY_Handle REPY_TupleGetIndexS32(REPY_Handle object, int index) {
    return PythonNative_Object_TupleGetIndexS32(object, index);
}

// Dict Operations:
RECOMP_EXPORT REPY_Handle REPY_CreateEmptyDict() {
    return PythonNative_Dict_Create(0, NULL);
}

RECOMP_EXPORT REPY_Handle REPY_CreateEmptyDict_SUH() {
    return PythonNative_Object_MakeSUH(PythonNative_Dict_Create(0, NULL));
}

RECOMP_EXPORT REPY_Handle REPY_CreateDict(u32 size, ...) {
    va_list va;
    va_start(va, size);
    REPY_Handle handle = PythonNative_Dict_Create(size, va);
    va_end(va);

    return handle;
}

RECOMP_EXPORT REPY_Handle REPY_CreateDict_SUH(u32 size, ...) {
    va_list va;
    va_start(va, size);
    REPY_Handle handle = PythonNative_Dict_Create(size, va);
    va_end(va);

    return PythonNative_Object_MakeSUH(handle);
}

RECOMP_EXPORT REPY_Handle REPY_DictGet(REPY_Handle dict, REPY_Handle key) {
    return PythonNative_Dict_Get(dict, key);
}

RECOMP_EXPORT REPY_Handle REPY_DictGet_CStr(REPY_Handle dict, char* key) {
    return PythonNative_Dict_Get_CStr(dict, key);
}

RECOMP_EXPORT void REPY_DictSet(REPY_Handle dict, REPY_Handle key, REPY_Handle value) {
    PythonNative_Dict_Set(dict, key, value);
}

RECOMP_EXPORT void REPY_DictSet_CStr(REPY_Handle dict, char* key, REPY_Handle value) {
    PythonNative_Dict_Set_CStr(dict, key, value);
}

RECOMP_EXPORT bool REPY_DictHas(REPY_Handle dict, REPY_Handle key) {
    return PythonNative_Dict_Has(dict, key);
}

RECOMP_EXPORT bool REPY_DictHas_CStr(REPY_Handle dict, char* key) {
    return PythonNative_Dict_Has_CStr(dict, key);
}

RECOMP_EXPORT void REPY_DictDel(REPY_Handle dict, REPY_Handle key) {
    PythonNative_Dict_Remove(dict, key);
}

RECOMP_EXPORT void REPY_DictDel_CStr(REPY_Handle dict, char* key) {
    PythonNative_Dict_Remove_CStr(dict, key);
}

// Execution:
RECOMP_EXPORT REPY_Handle REPY_Compile(REPY_Handle code, REPY_Handle identifier, REPY_Handle mode) {
    return PythonNative_Compile(code, identifier, mode);
}

RECOMP_EXPORT REPY_Handle REPY_CompileCStr(const char* code, const char* identifier, REPY_CodeMode mode) {
    return PythonNative_CompileCStr(code, identifier, mode);
}

RECOMP_EXPORT REPY_Handle REPY_CompileCStrN(const char* code, u32 len, const char* identifier, REPY_CodeMode mode) {
    return PythonNative_CompileCStrN(code, len, identifier, mode);
}

RECOMP_EXPORT bool REPY_Exec(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_Exec(code, global_scope, local_scope);
}

RECOMP_EXPORT bool REPY_ExecCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_ExecCStr(code, global_scope, local_scope);
}

RECOMP_EXPORT bool REPY_ExecCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_ExecCStrN(code, len, global_scope, local_scope);
}

RECOMP_EXPORT REPY_Handle REPY_Eval(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_Eval(code, global_scope, local_scope);
}

RECOMP_EXPORT REPY_Handle REPY_EvalCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_EvalCStr(code, global_scope, local_scope);
}

RECOMP_EXPORT REPY_Handle REPY_EvalCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope) {
    return PythonNative_EvalCStrN(code, len, global_scope, local_scope);
}

// Python Functions
RECOMP_EXPORT bool REPY_Call(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_Call(func, args, kwargs);
}

RECOMP_EXPORT REPY_Handle REPY_Call_Return(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_Call_Return(func, args, kwargs);
}

RECOMP_EXPORT bool REPY_CallAttr(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_CallAttr(func, name, args, kwargs);
}

RECOMP_EXPORT bool REPY_CallAttr_CStr(REPY_Handle func, char* name, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_CallAttr_CStr(func, name, args, kwargs);
}

RECOMP_EXPORT REPY_Handle REPY_CallAttr_Return(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_CallAttr_Return(func, name, args, kwargs);
}

RECOMP_EXPORT REPY_Handle REPY_CallAttr_CStr_Return(REPY_Handle func, char* name, REPY_Handle args, REPY_Handle kwargs) {
    return PythonNative_CallAttr_CStr_Return(func, name, args, kwargs);
}

// Exception Handling:
RECOMP_EXPORT bool REPY_IsErrorSet() {
    return PythonNative_IsErrorSet();
}

RECOMP_EXPORT REPY_Handle REPY_GetErrorType() {
    return PythonNative_GetErrorType();
}

RECOMP_EXPORT REPY_Handle REPY_GetErrorTrace() {
    return PythonNative_GetErrorTrace();
}

RECOMP_EXPORT REPY_Handle REPY_GetErrorValue() {
    return PythonNative_GetErrorValue();
}

RECOMP_EXPORT void REPY_ClearError() {
    PythonNative_ClearError();
}




