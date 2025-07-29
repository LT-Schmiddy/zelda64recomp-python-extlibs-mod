#ifndef __REPY_API__
#define __REPY_API__

#include "modding.h"
#include "global.h"
#include "recomputils.h"

/*! \file repy_api.h
    \version 0.1.4
    \brief The main header for Recomp External Python
 */

/**
 * @brief The mod id string for REPY.
 * 
 * The `recomp_py.h` imports all the functions and events needed for REPY, 
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

typedef unsigned int REPY_Handle;

typedef enum REPY_CodeMode {
    REPY_CODE_EXEC = 0,
    REPY_CODE_EVAL = 1,
    REPY_CODE_SINGLE = 2
} REPY_CodeMode;

typedef enum REPY_CHReturnType {
    REPY_CH_RETURN_FALSE = 0,
    REPY_CH_RETURN_TRUE = 1,
    REPY_CH_RETURN_WAS_COMPILED = 2,
    REPY_CH_RETURN_WAS_COMPILED_SUCCESSFULLY = 3,
    REPY_CH_RETURN_HANDLE = 4
} REPY_CHReturnType;

typedef struct REPY_IteratorHelper {
    REPY_Handle iter;
    u32 index;
    REPY_Handle curr;
    REPY_Handle py_scope;
    REPY_Handle var_name;
    bool _first_update;
} REPY_IteratorHelper;

#define REPY_NO_OBJECT 0

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

#define REPY_GLOBAL_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        bytecode_identifier = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_mode); \
    } \
} \

#define REPY_STATIC_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
static REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        bytecode_identifier = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_mode); \
    } \
} \

#define REPY_INLINE_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
static REPY_Handle bytecode_identifier = 0; \
if (bytecode_identifier == 0) { \
    /* bytecode_identifier = REPY_CompileCStr(code_str, __FILE_NAME__ ", in identifier '" #bytecode_identifier "' ", code_mode);*/ \
    char* iden_str = REPY_CodeSourceStrHelper(__FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
    bytecode_identifier = REPY_CompileCStr(code_str, (const char*)iden_str, code_mode); \
    recomp_free(iden_str); \
} \

// Python Object Flow Control
#define REPY_FOREACH_BLOCK(iter_identifier, py_object, py_scope, var_name) \
for (REPY_IteratorHelper* iter_identifier = REPY_IteratorHelper_Create(py_object, py_scope, var_name); REPY_IteratorHelper_Update(iter_identifier, true);)

#define REPY_FOREACH(iter_identifier, py_object) \
REPY_FOREACH_BLOCK(iter_identifier, py_object, 0, NULL)

#define REPY_FOREACH_CLEANUP_NOW(iter_identifier) \
REPY_IteratorHelper_Destroy(iter_identifier)


// FN - Overhead: 
#define REPY_FN_SETUP \
REPY_Handle _py_globals = REPY_CreateEmptyDict(); \
REPY_Handle _py_locals = _py_globals \

#define REPY_FN_SETUP_WITH_GLOBALS(globals) \
REPY_Handle _py_globals = globals; \
REPY_Handle _py_locals = REPY_CreateEmptyDict() \

#define REPY_FN_CLEANUP \
REPY_Release(_py_locals)

#define REPY_FN_RETURN \
REPY_Release(_py_locals); return


// FN - Execution:
#define REPY_FN_EXEC(bytecode_handle) \
REPY_Exec(bytecode_handle, _py_globals, _py_locals) 

#define REPY_FN_EXEC_CSTR(code_str) \
REPY_ExecCStr(code_str, _py_globals, _py_locals) 

#define REPY_FN_EXEC_CACHE(identifier, code_str) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EXEC, code_str) \
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
#define REPY_FN_EVAL_CSTR(code_str) \
REPY_EvalCStr(code, _py_globals, _py_locals) 

#define REPY_FN_EVAL_CSTR_BOOL(code_str) \
REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U32(code_str) \
REPY_CastU32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S32(code_str) \
REPY_CastS32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_F32(code_str) \
REPY_CastF32(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U64(code_str) \
REPY_CastU64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S64(code_str) \
REPY_CastS64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_F64(code_str) \
REPY_CastF64(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_STR(code_str) \
REPY_CastStr(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_BYTESTR(code_str) \
REPY_CastBytes(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

// FN - Eval Cache Block:
#define REPY_FN_EVAL_CACHE(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
REPY_Handle out_var = REPY_FN_EVAL(identifier)

#define REPY_FN_EVAL_CACHE_BOOL(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
bool out_var = REPY_FN_EVAL_BOOL(identifier)

#define REPY_FN_EVAL_CACHE_U32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
u32 out_var = REPY_FN_EVAL_U32(identifier)

#define REPY_FN_EVAL_CACHE_S32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
s32 out_var = REPY_FN_EVAL_S32(identifier)

#define REPY_FN_EVAL_CACHE_F32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
f32 out_var = REPY_FN_EVAL_F32(identifier)

#define REPY_FN_EVAL_CACHE_U64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
u64 out_var = REPY_FN_EVAL_U64(identifier)

#define REPY_FN_EVAL_CACHE_S64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
s64 out_var = REPY_FN_EVAL_S64(identifier)

#define REPY_FN_EVAL_CACHE_F64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
f64 out_var = REPY_FN_EVAL_F64(identifier)

#define REPY_FN_EVAL_CACHE_STR(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_STR(identifier)

#define REPY_FN_EVAL_CACHE_BYTESTR(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE(identifier, REPY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_BYTESTR(identifier)


// Scope Management - Modules:
#define REPY_FN_IMPORT(module_name) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateStr(module_name)), REPY_MakeSUH(REPY_ImportModule(module_name)))

// Scope Management - Primatives:
#define REPY_FN_GET(var_name) \
REPY_DictGet_CStr(_py_locals, var_name);

#define REPY_FN_SET(var_name, py_object) \
REPY_DictSet_CStr(_py_locals, var_name, py_object)

#define REPY_FN_GET_BOOL(var_name) \
REPY_CastBool(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_BOOL(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBool(value)))

#define REPY_FN_GET_U8(var_name) \
REPY_CastU8(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_U8(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU8(value)))

#define REPY_FN_GET_S8(var_name) \
REPY_CastS8(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_S8(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS8(value)))

#define REPY_FN_GET_U16(var_name) \
REPY_CastU16(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_U16(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU16(value)))

#define REPY_FN_GET_S16(var_name) \
REPY_CastS16(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_S16(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS16(value)))

#define REPY_FN_GET_U32(var_name) \
REPY_CastU32(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_U32(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU32(value)))

#define REPY_FN_GET_S32(var_name) \
REPY_CastS32(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_S32(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS32(value)))

#define REPY_FN_GET_F32(var_name) \
REPY_CastF32(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_F32(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateF32(value)))

#define REPY_FN_GET_U64(var_name) \
REPY_CastU64(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_U64(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU64(value)))

#define REPY_FN_GET_S64(var_name) \
REPY_CastS64(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_S64(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS64(value)))

#define REPY_FN_GET_F64(var_name) \
REPY_CastF64(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_F64(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateF64(value)))

// Scope Management - Strings
#define REPY_FN_GET_STR(var_name) \
REPY_CastStr(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, var_name)))

#define REPY_FN_SET_STR(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateStr(value)))

#define REPY_FN_SET_STR_N(var_name, value, len) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateStr(value, len)))


// Scope Management - Strings
#define REPY_FN_GET_BYTESTR(var_name) \
REPY_CastBytes(REPY_MakeSUH(REPY_DictGet_CStr(_py_locals, REPY_MakeSUH(REPY_CreateBytes(var_name)))))

#define REPY_FN_SET_BYTESTR(var_name, value) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBytes(value)))

#define REPY_FN_SET_BYTESTR_N(var_name, value, len) \
REPY_DictSet_CStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBytes(value, len)))

// Flow Control - If
#define REPY_FN_IF_INIT_BLOCK(bytecode_array_identifier, elif_count, py_expression) \
static REPY_Handle bytecode_array_identifier[elif_count + 1]; \
static bool bytecode_array_identifier ## _is_init = false; \
if (! bytecode_array_identifier ## _is_init) { \
    for (int i = 0; i < elif_count + 1; i++) { \
        bytecode_array_identifier[i] = 0; \
    } \
    bytecode_array_identifier ## _is_init = 1; \
} \
u32 bytecode_array_identifier ## _index = 0; \

#define REPY_FN_IF_STMT(bytecode_array_identifier, py_expression) \
if ( \
    REPY_CompileHelper( \
        &bytecode_array_identifier[bytecode_array_identifier ## _index], \
        py_expression, \
        __FILE_NAME__ ", in REPY_FN_IF with identifier '" #bytecode_array_identifier "' -> " #py_expression , \
        REPY_CODE_EVAL, \
        REPY_CH_RETURN_TRUE \
    )  && REPY_FN_EVAL_BOOL(bytecode_array_identifier[bytecode_array_identifier ## _index++]) \
) 

#define REPY_FN_IF(bytecode_array_identifier, elif_count, py_expression) \
REPY_FN_IF_INIT_BLOCK(bytecode_array_identifier, elif_count, py_expression) \
REPY_FN_IF_STMT(bytecode_array_identifier, py_expression) 

#define REPY_FN_ELIF(bytecode_array_identifier, py_expression) \
else REPY_FN_IF_STMT(bytecode_array_identifier, py_expression)

// Flow Control - Loops
#define REPY_FN_WHILE(bytecode_identifier, py_expression) \
REPY_INLINE_COMPILE_CACHE(bytecode_identifier, REPY_CODE_EVAL, py_expression); \
while (REPY_FN_EVAL_BOOL(bytecode_identifier))

#define REPY_FN_FOREACH(bytecode_identifier, var_name, py_expression) \
REPY_INLINE_COMPILE_CACHE(bytecode_identifier, REPY_CODE_EVAL, py_expression); \
REPY_FOREACH_BLOCK(bytecode_identifier ## _iter, REPY_MakeSUH(REPY_FN_EVAL(bytecode_identifier)), _py_locals, var_name)

#define REPY_FN_FOREACH_CLEANUP_NOW(bytecode_identifier) \
REPY_IteratorHelper_Destroy(bytecode_identifier ## _iter)

#define REPY_FN_FOR(bytecode_identifier, py_init_statement, py_eval_expression, py_after_statement) \
REPY_INLINE_COMPILE_CACHE(bytecode_identifier ## _init_statement, REPY_CODE_EXEC, py_init_statement); \
REPY_INLINE_COMPILE_CACHE(bytecode_identifier ## _eval_expression, REPY_CODE_EVAL, py_eval_expression); \
REPY_INLINE_COMPILE_CACHE(bytecode_identifier ## _after_statement, REPY_CODE_EXEC, py_after_statement); \
for ( \
    REPY_FN_EXEC(bytecode_identifier ## _init_statement); \
    REPY_FN_EVAL_BOOL(bytecode_identifier ## _eval_expression); \
    REPY_FN_EXEC(bytecode_identifier ## _after_statement) \
) 


// General:
REPY_IMPORT(void REPY_Release(REPY_Handle py_object));
REPY_IMPORT(REPY_Handle REPY_MakeSUH(REPY_Handle py_object));
REPY_IMPORT(bool REPY_IsValidHandle(REPY_Handle py_object));
REPY_IMPORT(bool REPY_GetSUH(REPY_Handle py_object));
REPY_IMPORT(void REPY_SetSUH(REPY_Handle py_object, bool value));
REPY_IMPORT(REPY_Handle REPY_CopyHandle(REPY_Handle py_object));

// Modules:
REPY_IMPORT(void REPY_LoadModule(const char* identifier, const char* code));
REPY_IMPORT(void REPY_LoadModuleN(const char* identifier, const char* code, u32 len));
REPY_IMPORT(REPY_Handle REPY_ImportModule(const char* identifier));

// Primatives
REPY_IMPORT(REPY_Handle REPY_CreateBool(bool value));
REPY_IMPORT(REPY_Handle REPY_CreateBool_SUH(bool value));
REPY_IMPORT(bool REPY_CastBool(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateU8(u8 value));
REPY_IMPORT(REPY_Handle REPY_CreateU8_SUH(u8 value));
REPY_IMPORT(u8 REPY_CastU8(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateS8(s8 value));
REPY_IMPORT(REPY_Handle REPY_CreateS8_SUH(s8 value));
REPY_IMPORT(s8 REPY_CastS8(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateU16(u16 value));
REPY_IMPORT(REPY_Handle REPY_CreateU16_SUH(u16 value));
REPY_IMPORT(u16 REPY_CastU16(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateS16(s16 value));
REPY_IMPORT(REPY_Handle REPY_CreateS16_SUH(s16 value));
REPY_IMPORT(s16 REPY_CastS16(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateU32(u32 value));
REPY_IMPORT(REPY_Handle REPY_CreateU32_SUH(u32 value));
REPY_IMPORT(u32 REPY_CastU32(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateS32(s32 value));
REPY_IMPORT(REPY_Handle REPY_CreateS32_SUH(s32 value));
REPY_IMPORT(s32 REPY_CastS32(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateF32(f32 value));
REPY_IMPORT(REPY_Handle REPY_CreateF32_SUH(f32 value));
REPY_IMPORT(f32 REPY_CastF32(REPY_Handle object));

// 64-bit Primatives
REPY_IMPORT(REPY_Handle REPY_CreateU64(u64 value));
REPY_IMPORT(REPY_Handle REPY_CreateU64_SUH(u64 value));
REPY_IMPORT(u64 REPY_CastU64(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateS64(s64 value));
REPY_IMPORT(REPY_Handle REPY_CreateS64_SUH(s64 value));
REPY_IMPORT(s64 REPY_CastS64(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateF64(f64 value));
REPY_IMPORT(REPY_Handle REPY_CreateF64_SUH(f64 value));
REPY_IMPORT(f64 REPY_CastF64(REPY_Handle object));

// Strings:
REPY_IMPORT(REPY_Handle REPY_CreateStr(const char* string));
REPY_IMPORT(REPY_Handle REPY_CreateStr_SUH(const char* string));
REPY_IMPORT(REPY_Handle REPY_CreateStrN(const char* string, u32 len) );
REPY_IMPORT(REPY_Handle REPY_CreateStrN_SUH(const char* string, u32 len) );
REPY_IMPORT(char* REPY_CastStr(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_CreateByteStr(const char* string));
REPY_IMPORT(REPY_Handle REPY_CreateByteStr_SUH(const char* string));
REPY_IMPORT(REPY_Handle REPY_CreateByteStrN(const char* string, u32 len));
REPY_IMPORT(REPY_Handle REPY_CreateByteStrN_SUH(const char* string, u32 len));
REPY_IMPORT(char* REPY_CastByteStr(REPY_Handle object));

// Memcpy:
REPY_IMPORT(REPY_Handle REPY_MemcpyToBytes(void* src, u32 len, u32 reverse));
REPY_IMPORT(u32 REPY_MemcpyFromBytes(void* src, u32 len, u32 reverse, REPY_Handle bytes_obj));
REPY_IMPORT(void* REPY_AllocAndCopyBytes(u32 reverse, REPY_Handle bytes_obj, u32* write_size));
REPY_IMPORT(REPY_Handle REPY_MemcpyToByteArray(void* src, u32 len, u32 reverse));
REPY_IMPORT(u32 REPY_MemcpyFromByteArray(void* src, u32 len, u32 reverse, REPY_Handle bytes_obj));
REPY_IMPORT(void* REPY_AllocAndCopyByteArray(u32 reverse, REPY_Handle bytes_obj, u32* write_size));

// Indexing and Slicing:
REPY_IMPORT(u32 REPY_Len(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_GetIndex(REPY_Handle object, int index));

// Iteration
REPY_IMPORT(REPY_Handle REPY_Iter(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_Next(REPY_Handle iterator, REPY_Handle default_obj, u32 process_stop_iteration));

// Tuple:
REPY_IMPORT(REPY_Handle REPY_CreateTuple(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreateTuple_SUH(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreatePair(REPY_Handle key, REPY_Handle value));
REPY_IMPORT(REPY_Handle REPY_CreatePair_SUH(REPY_Handle key, REPY_Handle value));

// Dicts:
REPY_IMPORT(REPY_Handle REPY_CreateEmptyDict());
REPY_IMPORT(REPY_Handle REPY_CreateEmptyDict_SUH());
REPY_IMPORT(REPY_Handle REPY_CreateDict(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreateDict_SUH(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_DictGet(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(REPY_Handle REPY_DictGet_CStr(REPY_Handle dict, char* key));
REPY_IMPORT(void REPY_DictSet(REPY_Handle dict, REPY_Handle key, REPY_Handle value));
REPY_IMPORT(void REPY_DictSet_CStr(REPY_Handle dict, char* key, REPY_Handle value));
REPY_IMPORT(bool REPY_DictHas(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(bool REPY_DictHas_CStr(REPY_Handle dict, char* key));
REPY_IMPORT(void REPY_DictDel(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(void REPY_DictDel_CStr(REPY_Handle dict, char* key));

// Execution:
REPY_IMPORT(REPY_Handle REPY_Compile(REPY_Handle code, REPY_Handle identifier, REPY_Handle mode));
REPY_IMPORT(REPY_Handle REPY_CompileCStr(const char* code, const char* identifier, REPY_CodeMode mode));
REPY_IMPORT(REPY_Handle REPY_CompileCStrN(const char* code, u32 len, const char* identifier, REPY_CodeMode mode));
REPY_IMPORT(bool REPY_Exec(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope));
REPY_IMPORT(bool REPY_ExecCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope));
REPY_IMPORT(bool REPY_ExecCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope));
REPY_IMPORT(REPY_Handle REPY_Eval(REPY_Handle code, REPY_Handle global_scope, REPY_Handle local_scope));
REPY_IMPORT(REPY_Handle REPY_EvalCStr(const char* code, REPY_Handle global_scope, REPY_Handle local_scope));
REPY_IMPORT(REPY_Handle REPY_EvalCStrN(const char* code, u32 len, REPY_Handle global_scope, REPY_Handle local_scope));

// Python Function Calls
REPY_IMPORT(bool REPY_Call(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
REPY_IMPORT(REPY_Handle REPY_CallReturn(REPY_Handle func, REPY_Handle args, REPY_Handle kwargs));
REPY_IMPORT(bool REPY_CallAttr(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs));
REPY_IMPORT(bool REPY_CallAttr_CStr(REPY_Handle func, char* name, REPY_Handle args, REPY_Handle kwargs));
REPY_IMPORT(REPY_Handle REPY_CallAttr_Return(REPY_Handle func, REPY_Handle name, REPY_Handle args, REPY_Handle kwargs));
REPY_IMPORT(REPY_Handle REPY_CallAttr_CStr_Return(REPY_Handle func, char* name, REPY_Handle args, REPY_Handle kwargs));

// Error Handling
REPY_IMPORT(bool REPY_IsErrorSet());
REPY_IMPORT(REPY_Handle REPY_GetErrorType());
REPY_IMPORT(REPY_Handle REPY_GetErrorTrace());
REPY_IMPORT(REPY_Handle REPY_GetErrorValue());
REPY_IMPORT(void REPY_ClearError());


// Helpers:
REPY_IMPORT(u32 REPY_CompileHelper(REPY_Handle* handle_ptr, const char* code_str, const char* identifier, REPY_CodeMode code_mode, REPY_CHReturnType return_type));
REPY_IMPORT(REPY_IteratorHelper* REPY_IteratorHelper_Create(REPY_Handle py_object, REPY_Handle py_scope, const char* var_name));
REPY_IMPORT(void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper));
REPY_IMPORT(bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, bool auto_destroy));
REPY_IMPORT(char* REPY_CodeSourceStrHelper(char* filename, char* function_name, u32 line_number, char* identifier));

#endif