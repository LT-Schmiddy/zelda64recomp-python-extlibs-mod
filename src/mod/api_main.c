#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"

#include "extlib_functions.h"

RECOMP_DECLARE_EVENT(REPY_OnInit(int success));

RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int py_init = PythonNative_Init(mod_folder);
    recomp_free((void*)mod_folder);

    if (py_init == 1) {
        recomp_printf("Python interpreter initialized successfully.\n");
    } else {
        recomp_printf("There was an error initializing the Python interpreter.\n");
    }

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

// Modules:
RECOMP_EXPORT void REPY_LoadModule(const char* identifier, const char* code) {
    PythonNative_LoadModule(identifier, code);
}

RECOMP_EXPORT void REPY_LoadModuleN(const char* identifier, const char* code, u32 len) {
    PythonNative_LoadModuleN(identifier, code, len);
}

// Primatives:
RECOMP_EXPORT PyObjectHandle REPY_CreateBool(bool value) {
    return PythonNative_Object_CreateBool(value);
}

RECOMP_EXPORT bool REPY_CastBool(PyObjectHandle object) {
    return PythonNative_Object_CastBool(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateU32(u32 value) {
    return PythonNative_Object_CreateU32(value);
}

RECOMP_EXPORT u32 REPY_CastU32(PyObjectHandle object) {
    return PythonNative_Object_CastU32(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateS32(s32 value) {
    return PythonNative_Object_CreateS32(value);
}

RECOMP_EXPORT s32 REPY_CastS32(PyObjectHandle object) {
    return PythonNative_Object_CastS32(object);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateF32(f32 value) {
    return PythonNative_Object_CreateF32(value);
}

RECOMP_EXPORT f32 REPY_CastF32(PyObjectHandle object) {
    return PythonNative_Object_CastF32(object);
}

// Strings:
RECOMP_EXPORT PyObjectHandle REPY_CreateStr(const char* string) {
    return PythonNative_Object_CreateStr(string);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateStrN(const char* string, u32 len) {
    return PythonNative_Object_CreateStrN(string, len);
}

RECOMP_EXPORT char* REPY_CastStr(PyObjectHandle object) {
    u32 len = PythonNative_Object_CastStr_Prepare(object);
    char* retVal = recomp_alloc(len);
    PythonNative_Object_CastStr_Copy(len, retVal);
    return retVal;
}

RECOMP_EXPORT PyObjectHandle REPY_CreateBytes(const char* string) {
    return PythonNative_Object_CreateBytes(string);
}

RECOMP_EXPORT PyObjectHandle REPY_CreateBytesN(const char* string, u32 len) {
    return PythonNative_Object_CreateBytesN(string, len);
}

RECOMP_EXPORT char* REPY_CastBytes(PyObjectHandle object) {
    u32 len = PythonNative_Object_CastBytes_Prepare(object);
    char* retVal = recomp_alloc(len);
    PythonNative_Object_CastBytes_Copy(len, retVal);
    return retVal;
}

// Dict Operations:
RECOMP_EXPORT PyObjectHandle REPY_CreateDict() {
    return PythonNative_Dict_Create();
}

RECOMP_EXPORT PyObjectHandle REPY_DictGet(PyObjectHandle dict, PyObjectHandle key) {
    return PythonNative_Dict_Get(dict, key);
}

RECOMP_EXPORT void REPY_DictSet(PyObjectHandle dict, PyObjectHandle key, PyObjectHandle value) {
    PythonNative_Dict_Set(dict, key, value);
}

RECOMP_EXPORT PyObjectHandle REPY_DictHas(PyObjectHandle dict, PyObjectHandle key) {
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