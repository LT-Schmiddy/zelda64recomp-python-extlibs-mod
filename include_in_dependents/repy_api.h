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

// ========== Macros: ==========
#define REPY_FN_SETUP \
PyObjectHandle _py_globals = REPY_CreateDict(); \
PyObjectHandle _py_locals = _py_globals \


#define REPY_FN_SETUP_WITH_GLOBALS(globals) \
PyObjectHandle _py_globals = globals; \
PyObjectHandle _py_locals = REPY_CreateDict() \


#define REPY_FN_RETURN \
REPY_Release(_py_locals); return


#define REPY_FN_EXEC(identifier, code) \
static PyObjectHandle identifier ## _bytecode = 0; \
if (identifier ## _bytecode == 0) { \
    identifier ## _bytecode = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier" #identifier " ", PY_CODE_EXEC); \
} \
u32 identifier ## success = REPY_Exec(identifier ## _bytecode, _py_globals, _py_locals) 


#define REPY_FN_EXEC_INLINE(code) \
REPY_ExecCStr(code, _py_globals, _py_locals) 


#define REPY_FN_EVAL(identifier, code, out_var) \
static PyObjectHandle identifier ## _bytecode = 0; \
if (identifier ## _bytecode == 0) { \
    identifier ## _bytecode = REPY_CompileCStr(code, __FILE_NAME__ ", in identifier" #identifier " ", PY_CODE_EVAL); \
} \
PyObjectHandle out_var = REPY_Eval(identifier ## _bytecode, _py_globals, _py_locals)


#define REPY_FN_EVAL_INLINE(code) \
REPY_EvalCStr(code, _py_globals, _py_locals) 

// Scope Management:
#define REPY_FN_SCOPE_GET(var_name) \
REPY_DictGet(_py_locals, py_object);

#define REPY_FN_SCOPE_SET_BOOL(var_name, value) \
REPY_DictSet(_py_locals, REPY_MakeSUH(REPY_CreateBool(value)));

#define REPY_FN_SCOPE_SET(var_name, py_object) \
REPY_DictSet(_py_locals, py_object);


// ========== API: ==========
// Events:
#define REPY_ON_INIT RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnInit)

// General:
REPY_IMPORT(void REPY_Release(PyObjectHandle py_object));
REPY_IMPORT(PyObjectHandle REPY_MakeSUH(PyObjectHandle py_object));

// Modules:
REPY_IMPORT(void REPY_LoadModule(const char* identifier, const char* code));
REPY_IMPORT(void REPY_LoadModuleN(const char* identifier, const char* code, u32 len));

// Primatives
REPY_IMPORT(PyObjectHandle REPY_CreateBool(bool value));
REPY_IMPORT(bool REPY_CastBool(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateU32(u32 value));
REPY_IMPORT(u32 REPY_CastU32(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateS32(s32 value));
REPY_IMPORT(s32 REPY_CastS32(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateF32(f32 value));
REPY_IMPORT(f32 REPY_CastF32(PyObjectHandle object));

// Strings:
REPY_IMPORT(PyObjectHandle REPY_CreateStr(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateStrN(const char* string, u32 len) );
REPY_IMPORT(char* REPY_CastStr(PyObjectHandle object));
REPY_IMPORT(PyObjectHandle REPY_CreateBytes(const char* string));
REPY_IMPORT(PyObjectHandle REPY_CreateBytesN(const char* string, u32 len));
REPY_IMPORT(char* REPY_CastBytes(PyObjectHandle object));

// Dicts:
REPY_IMPORT(PyObjectHandle REPY_CreateDict());
REPY_IMPORT(PyObjectHandle REPY_DictGet(PyObjectHandle dict, PyObjectHandle key));
REPY_IMPORT(void REPY_DictSet(PyObjectHandle dict, PyObjectHandle key, PyObjectHandle value));
REPY_IMPORT(PyObjectHandle REPY_DictHas(PyObjectHandle dict, PyObjectHandle key));
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

#endif