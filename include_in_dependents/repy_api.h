#ifndef __REPY_API__
#define __REPY_API__

#include "modding.h"
#include "global.h"

/*! \file repy_api.h
    \version 1.0.0
    \brief The main header for Recomp External Python
 */

/**
 * @brief The mod id string for REPY.
 * 
 * The `recomp_py.h` imports all the functions and events needed for RecompPy, 
 * so you probably won't need to use this directly.
 */
#define REPY_MOD_ID_STR "MM_Recomp_External_Python_API"

#ifdef DOXYGEN
#define REPY_IMPORT(func) func

#elif RECOMP_PY_BUILD_MODE
#define REPY_IMPORT(func) func

#else 
#define REPY_IMPORT(func) RECOMP_IMPORT(REPY_MOD_ID_STR, func)

#ifndef __FILE_NAME__
#define __FILE_NAME__ "Unknown Source File"
#endif

#endif

typedef int PyObjectHandle;

typedef enum PythonCodeMode {
    PY_CODE_EXEC = 0,
    PY_CODE_EVAL = 1,
    PY_CODE_SINGLE = 2
} PythonCodeMode;

// ========== API: ==========
// Events:
#define REPY_ON_LOAD_MODULES RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnLoadModules)
#define REPY_ON_MAKE_GLOBAL_CACHES RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnMakeGlobalCaches)
#define REPY_ON_INIT RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnInit)


// ========== Macros: ==========
// Startup Code Caching.

#ifdef REPY_SILENCE_INCBIN_SQUIGGLES
#define REPY_INCBIN_PYFILE(identifier, filename)      \
    extern u8 identifier[];                           \
    extern u8 identifier##_end[]

#else
#define REPY_INCBIN_PYFILE(identifier, filename)      \
    asm(".pushsection .rodata\n"                      \
        "\t.globl " #identifier "\n"                  \
        "\t.type " #identifier ", @object\n"          \
        "\t.balign 8\n"                               \
        #identifier ":\n"                             \
        "\t.incbin \"" filename "\"\n"                \
        "\t.globl " #identifier "_end\n"              \
        #identifier "_end:\n"                         \
        "\t.popsection\n");                           \
    extern u8 identifier[];                           \
    extern u8 identifier##_end[]
#endif

#define REPY_INCBIN_MODULE(module_name, file_name) \
REPY_INCBIN_PYFILE(module_name ## _code, file_name); \
REPY_ON_LOAD_MODULES void _construct_module_ ## module_name (int success) { \
    if (success) { \
        REPY_LoadModuleN(#module_name, (const char*)module_name ## _code, (u32) (module_name ## _code_end - module_name ## _code)); \
    } \
} \

#define REPY_GLOBAL_CODE_CACHE(bytecode_identifier, code_type, code_str) \
PyObjectHandle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        bytecode_identifier = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_type); \
    } \
} \

#define REPY_STATIC_CODE_CACHE(bytecode_identifier, code_mode, code_str) \
static PyObjectHandle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        bytecode_identifier = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_mode); \
    } \
} \

// FN - Overhead: 
#define REPY_FN_SETUP \
PyObjectHandle _py_globals = REPY_CreateEmptyDict(); \
PyObjectHandle _py_locals = _py_globals \

#define REPY_FN_SETUP_WITH_GLOBALS(globals) \
PyObjectHandle _py_globals = globals; \
PyObjectHandle _py_locals = REPY_CreateEmptyDict() \

#define REPY_FN_CLEANUP \
REPY_Release(_py_locals)

#define REPY_FN_RETURN \
REPY_Release(_py_locals); return

#define REPY_FN_CODE_CACHE(bytecode_identifier, code_mode, code_str) \
static PyObjectHandle bytecode_identifier = 0; \
if (bytecode_identifier == 0) { \
    bytecode_identifier = REPY_CompileCStr(code_str, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_mode); \
} \

// FN - Execution:
#define REPY_FN_EXEC(bytecode_handle) \
REPY_Exec(bytecode_handle, _py_globals, _py_locals) 

#define REPY_FN_EXEC_CSTR(code_str) \
REPY_ExecCStr(code_str, _py_globals, _py_locals) 

#define REPY_FN_EXEC_BLOCK(identifier, code_str) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EXEC, code_str) \
u32 identifier ## _success = REPY_FN_EXEC(identifier) 

// FN - Eval Bytecode:
#define REPY_FN_EVAL(bytecode_handle) \
REPY_Eval(bytecode_handle, _py_globals, _py_locals) 

#define REPY_FN_EVAL_BOOL(bytecode_handle) \
REPY_CastBool(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_U32(bytecode_handle) \
REPY_CastU32(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_S32(bytecode_handle) \
REPY_CastS32(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_F32(bytecode_handle) \
REPY_CastF32(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_U64(bytecode_handle) \
REPY_CastU64(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_S64(bytecode_handle) \
REPY_CastS64(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_F64(bytecode_handle) \
REPY_CastF64(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_STR(bytecode_handle) \
REPY_CastStr(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_BYTESTR(bytecode_handle) \
REPY_CastBytes(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

// FN - Eval CStr
#define REPY_FN_EVAL_CSTR(code) \
REPY_EvalCStr(code, _py_globals, _py_locals) 

#define REPY_FN_EVAL_CSTR_BOOL(code) \
REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U32(code) \
REPY_CastU32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S32(code) \
REPY_CastS32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_F32(code) \
REPY_CastF32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U64(code) \
REPY_CastU64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S64(code) \
REPY_CastS64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_F64(code) \
REPY_CastF64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_STR(code) \
REPY_CastStr(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_BYTESTR(code) \
REPY_CastBytes(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

// FN - Eval Cache Block:
#define REPY_FN_EVAL_BLOCK(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
PyObjectHandle out_var = REPY_FN_EVAL(identifier)

#define REPY_FN_EVAL_BLOCK_BOOL(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
bool out_var = REPY_FN_EVAL_BOOL(identifier)

#define REPY_FN_EVAL_BLOCK_U32(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
u32 out_var = REPY_FN_EVAL_U32(identifier)

#define REPY_FN_EVAL_BLOCK_S32(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
s32 out_var = REPY_FN_EVAL_S32(identifier)

#define REPY_FN_EVAL_BLOCK_F32(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
f32 out_var = REPY_FN_EVAL_F32(identifier)

#define REPY_FN_EVAL_BLOCK_U64(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
u64 out_var = REPY_FN_EVAL_U64(identifier)

#define REPY_FN_EVAL_BLOCK_S64(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
s64 out_var = REPY_FN_EVAL_S64(identifier)

#define REPY_FN_EVAL_BLOCK_F64(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
f64 out_var = REPY_FN_EVAL_F64(identifier)

#define REPY_FN_EVAL_BLOCK_STR(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_STR(identifier)

#define REPY_FN_EVAL_BLOCK_BYTESTR(identifier, code_str, out_var) \
REPY_FN_CODE_CACHE(identifier, PY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_BYTESTR(identifier)


// Scope Management - Modules:
#define REPY_FN_IMPORT(module_name) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(module_name)), REPY_MakeSUH(REPY_ImportModule(module_name)))

// Scope Management - Primatives:
#define REPY_FN_GET(var_name) \
REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)));

#define REPY_FN_SET(var_name, py_object) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), py_object)

#define REPY_FN_GET_BOOL(var_name) \
REPY_CastBool(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_BOOL(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateBool(value)))

#define REPY_FN_GET_U32(var_name) \
REPY_CastU32(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_U32(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateU32(value)))

#define REPY_FN_GET_S32(var_name) \
REPY_CastS32(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_S32(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateS32(value)))

#define REPY_FN_GET_F32(var_name) \
REPY_CastF32(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_F32(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateF32(value)))

#define REPY_FN_GET_U64(var_name) \
REPY_CastU64(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_U64(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateU64(value)))

#define REPY_FN_GET_S64(var_name) \
REPY_CastS64(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_S64(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateS64(value)))

#define REPY_FN_GET_F64(var_name) \
REPY_CastF64(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_F64(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateF64(value)))

// Scope Management - Strings
#define REPY_FN_GET_STR(var_name) \
REPY_CastStr(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)))))

#define REPY_FN_SET_STR(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateStr(value)))

#define REPY_FN_SET_STR_N(var_name, value, len) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateStr(value, len)))


// Scope Management - Strings
#define REPY_FN_GET_BYTESTR(var_name) \
REPY_CastBytes(REPY_MakeSUH(REPY_DictGet(_py_locals, REPY_MakeSUH(REPY_CreateBytes(var_name)))))

#define REPY_FN_SET_BYTESTR(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateBytes(value)))

#define REPY_FN_SET_BYTESTR_N(var_name, value, len) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(var_name)), REPY_MakeSUH(REPY_CreateBytes(value, len)))


// General:
REPY_IMPORT(void REPY_Release(PyObjectHandle py_object));
REPY_IMPORT(PyObjectHandle REPY_MakeSUH(PyObjectHandle py_object));
REPY_IMPORT(bool REPY_IsValidHandle(PyObjectHandle py_object));
REPY_IMPORT(bool REPY_GetSUH(PyObjectHandle py_object));
REPY_IMPORT(void REPY_SetSUH(PyObjectHandle py_object, bool value));
REPY_IMPORT(PyObjectHandle REPY_CopyHandle(PyObjectHandle py_object));

// Modules:
REPY_IMPORT(void REPY_LoadModule(const char* identifier, const char* code));
REPY_IMPORT(void REPY_LoadModuleN(const char* identifier, const char* code, u32 len));
REPY_IMPORT(PyObjectHandle REPY_ImportModule(const char* identifier));

// Primatives
REPY_IMPORT(PyObjectHandle REPY_CreateBool(bool value));
REPY_IMPORT(PyObjectHandle REPY_CreateBool_SUH(bool value));
REPY_IMPORT(bool REPY_CastBool(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateU32(u32 value));
REPY_IMPORT(PyObjectHandle REPY_CreateU32_SUH(u32 value));
REPY_IMPORT(u32 REPY_CastU32(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateS32(s32 value));
REPY_IMPORT(PyObjectHandle REPY_CreateS32_SUH(s32 value));
REPY_IMPORT(s32 REPY_CastS32(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateF32(f32 value));
REPY_IMPORT(PyObjectHandle REPY_CreateF32_SUH(f32 value));
REPY_IMPORT(f32 REPY_CastF32(PyObjectHandle object));

// 64-bit Primatives
REPY_IMPORT(PyObjectHandle REPY_CreateU64(u64 value));
REPY_IMPORT(PyObjectHandle REPY_CreateU64_SUH(u64 value));
REPY_IMPORT(u64 REPY_CastU64(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateS64(s64 value));
REPY_IMPORT(PyObjectHandle REPY_CreateS64_SUH(s64 value));
REPY_IMPORT(s64 REPY_CastS64(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateF64(f64 value));
REPY_IMPORT(PyObjectHandle REPY_CreateF64_SUH(f64 value));
REPY_IMPORT(f64 REPY_CastF64(PyObjectHandle object));

// Strings:
REPY_IMPORT(PyObjectHandle REPY_CreateStr(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateStr_SUH(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateStrN(const char* string, u32 len) );
REPY_IMPORT(PyObjectHandle REPY_CreateStrN_SUH(const char* string, u32 len) );
REPY_IMPORT(char* REPY_CastStr(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateByteStr(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateByteStr_SUH(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateByteStrN(const char* string, u32 len));
REPY_IMPORT(PyObjectHandle REPY_CreateByteStrN_SUH(const char* string, u32 len));
REPY_IMPORT(char* REPY_CastByteStr(PyObjectHandle object));

// Tuple:
REPY_IMPORT(PyObjectHandle REPY_CreateTuple(u32 size, ...));
REPY_IMPORT(PyObjectHandle REPY_CreateTuple_SUH(u32 size, ...));
REPY_IMPORT(PyObjectHandle REPY_CreatePair(PyObjectHandle key, PyObjectHandle value));
REPY_IMPORT(PyObjectHandle REPY_CreatePair_SUH(PyObjectHandle key, PyObjectHandle value));
REPY_IMPORT(PyObjectHandle REPY_TupleGetMember(PyObjectHandle tuple, int index));

// Dicts:
REPY_IMPORT(PyObjectHandle REPY_CreateEmptyDict());
REPY_IMPORT(PyObjectHandle REPY_CreateEmptyDict_SUH());
REPY_IMPORT(PyObjectHandle REPY_CreateDict(u32 size, ...));
REPY_IMPORT(PyObjectHandle REPY_CreateDict_SUH(u32 size, ...));
REPY_IMPORT(PyObjectHandle REPY_DictGet(PyObjectHandle dict, PyObjectHandle key));
REPY_IMPORT(void REPY_DictSet(PyObjectHandle dict, PyObjectHandle key, PyObjectHandle value));
REPY_IMPORT(bool REPY_DictHas(PyObjectHandle dict, PyObjectHandle key));
REPY_IMPORT(void REPY_DictDel(PyObjectHandle dict, PyObjectHandle key));

// Execution:
REPY_IMPORT(PyObjectHandle REPY_Compile(PyObjectHandle code, PyObjectHandle identifier, PyObjectHandle mode));
REPY_IMPORT(PyObjectHandle REPY_CompileCStr(const char* code, const char* identifier, PythonCodeMode mode));
REPY_IMPORT(PyObjectHandle REPY_CompileCStrN(const char* code, u32 len, const char* identifier, PythonCodeMode mode));
REPY_IMPORT(bool REPY_Exec(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope));
REPY_IMPORT(bool REPY_ExecCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope));
REPY_IMPORT(bool REPY_ExecCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope));
REPY_IMPORT(PyObjectHandle REPY_Eval(PyObjectHandle code, PyObjectHandle global_scope, PyObjectHandle local_scope));
REPY_IMPORT(PyObjectHandle REPY_EvalCStr(const char* code, PyObjectHandle global_scope, PyObjectHandle local_scope));
REPY_IMPORT(PyObjectHandle REPY_EvalCStrN(const char* code, u32 len, PyObjectHandle global_scope, PyObjectHandle local_scope));

// Python Function Calls
REPY_IMPORT(bool REPY_Call(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs));
REPY_IMPORT(PyObjectHandle REPY_CallReturn(PyObjectHandle func, PyObjectHandle args, PyObjectHandle kwargs));
REPY_IMPORT(bool REPY_CallAttr(PyObjectHandle func, char* name, PyObjectHandle args, PyObjectHandle kwargs));
REPY_IMPORT(PyObjectHandle REPY_CallAttrReturn(PyObjectHandle func, char* name, PyObjectHandle args, PyObjectHandle kwargs));

// Error Handling
REPY_IMPORT(bool REPY_IsErrorSet());
REPY_IMPORT(PyObjectHandle REPY_GetErrorType());
REPY_IMPORT(PyObjectHandle REPY_GetErrorTrace());
REPY_IMPORT(PyObjectHandle REPY_GetErrorValue());
REPY_IMPORT(void REPY_ClearError());


#endif