#ifndef __REPY_API__
#define __REPY_API__

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

/*! \file repy_api.h
    \version 1.0.0
    \brief The main header for Recomp External Python. Header version 1.
    \
 */

/**
 * @brief The mod id string for REPY.
 * 
 * The `recomp_py.h` imports all the functions and events needed for REPY, 
 * so you probably won't need to use this directly.
 */
#define REPY_MOD_ID_STR "MM_RecompExternalPython_API"

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
/** \defgroup repy_types Types
 * \brief All of the C types that REPY defines.
 *  @{
 */

/**
 * @brief Represents a Python object in REPY API functions. 
 * 
 * Technically, the handle is a unsigned integer value between 0x00000001 and 0xFFFFFFFF, inclusive. The REPY external library
 * will look up the corresponding Python object from an internally managed slot map. A handle is considered 'valid'
 * if the value is assigned to a Python object. 
 * 
 * In terms of Python's reference-count-based garbage collection, each valid handle counts as a single reference 
 * to an object, and multiple handles can be mapped to a single Python object. Once created, a handle must be released
 * to remove that reference, either by using `REPY_Release` or flagging the handle as Single-Use (more on that below). 
 * If you need multiple references to a Python object, you can get a new handle to the same object using `REPY_CopyHandle`.
 * With the exception of `REPY_MakeSUH`, any REPY API function that returns a REPY_Handle will be creating a new handle. 
 * Failure to release handles will result in resource/memory leaks. 
 * 
 * The handle value of `REPY_NO_OBJECT` (the numerical value 0) is a special case, and represents the absense of any Python object. 
 * Not that this is different from Python's `None`, which is itself a Python object. If an API function with `REPY_Handle`
 * as the return type returns `REPY_NO_OBJECT`, that will generally mean a Python error has occured.
 * 
 * There are a few cases where a REPY_Handle of `REPY_NO_OBJECT` is acceptable as a function argument, indicating that the argument
 * is not used. These arguments are named with the `_nullable` suffix.
 * 
 * Handles can be flagged as Single-Use, which means that the handle will be immediately released after it's next use in a REPY
 * API function. This is primarily to allow nesting calls to REPY API functions without incurring resource/memory leaks. Any handle
 * can be flagged as Single-Use using `REPY_MakeSUH` or `REPY_SetSUH`, and the Single-Use status can be checked with `REPY_GetSUH`.
 * Several common REPY API functions have alternative versions that automatically return a Single Use handle. These functions will
 * have the suffix `_SUH` in their names.
 * 
 * Not that the only functions that won't release a Single-Use handle are ones meant to manipulate the handles themselves:
 * `REPY_MakeSUH`, `REPY_IsValidHandle`, `REPY_GetSUH`, and `REPY_SetSUH`. Their REPY_Handle argument names have the suffix `_no_release`
 * to reflect this.
 * 
 */
typedef unsigned int REPY_Handle;

/**
 * @brief Represents the absence of a Python object in REPY API functions.
 * 
 * A more readable alternative to simply entering 0.
 */
#define REPY_NO_OBJECT 0

/**
 * @brief Used to set the type of code-string being compiled, in line with how Python's
 * built-in `compile` function operates.
 * 
 * Used with `REPY_CompileCStr` and `REPY_CompileCStr`. `REPY_Compile` accepts a REPY_Handle string argument instead.
 * 
 */
typedef enum REPY_CodeMode {
    REPY_CODE_EXEC = 0,
    REPY_CODE_EVAL = 1,
    REPY_CODE_SINGLE = 2
} REPY_CodeMode;

/**
 * @brief Helper object used to when iterating through Python objects in loops in C code.
 * 
 * These objects are primarily used as part of the the `REPY_FOREACH` and `REPY_FN_FOREACH_CACHE` macros,
 * which simulate the behavior of Python's own `for` loops. You can use them manually as well.
 * 
 * The lifetime of each REPY_Handle member is managed by IteratorHelper. Do not release them manually.
 * 
 */
typedef struct REPY_IteratorHelper {
    REPY_Handle iter; ///< Handle for the Python iterator object. 
    u32 index; ///< The index of the current object from the iterator.
    REPY_Handle curr; ///< Handle of the current object from the iterator. If you need to access this object outside of the current iteration, use `REPY_CopyHandle` to get a new handle.
    REPY_Handle py_scope; ///< If this handle != 0, then the `curr` object will be added to this scope with a variable name set by `var_name`.
    REPY_Handle var_name; ///< The variable name that will be used for `curr` when added to `py_scope`, if `py_scope` is not 0.
    bool _first_update; ///< Internal flag used to determine if the iterator has been updated for the first time.
} REPY_IteratorHelper;

/**
 * @brief Helper object used to cache Python expressions as bytecode, so that they don't need to be re-parsed and compiled every time they're run.
 * 
 * Used as part of the macro `REPY_FN_IF_CACHE_INIT_BLOCK`, and is used with `REPY_FN_IF_STMT_CACHE`, `REPY_FN_IF_CACHE` and `REPY_FN_ELIF_CACHE`.
 * Generally initialized as a `static` variable, so that that the compiled bytecode is is preserverd between uses.
 * 
 * The chain is a singly-linked list with the bytecode for each Python expression from a `REPY_IfStmtHelper_Step` call. Each link is constructed the 
 * that step is called.
 * 
 */
typedef struct REPY_IfStmtChain {
    REPY_Handle eval_expression_bytecode; ///< The bytecode for the Python expression to evaluate.
    struct REPY_IfStmtChain* next; ///< Pointer to the next link in the chain.
} REPY_IfStmtChain;

/**
 * @brief Helper used to step through a `REPY_IfStmtChain` while it's being evaluated.
 * 
 */
typedef struct REPY_IfStmtHelper {
    u32 index; ///< The number of links down the chain we've gone.
    REPY_IfStmtChain** root; ///< The start of the chain. A double pointer is used so that, the the chain doesn't exist yet, it can be initialized on the first call of `REPY_IfStmtHelper_Step`.
    REPY_IfStmtChain* curr; ///< The most recently evaluated link in the chain.
    bool _first_step; ///< ///< Internal flag used to determine if the helper has been stepped for the first time.
} REPY_IfStmtHelper;

/** @}*/

/** \defgroup repy_events Events
 *  @{
 */

/**
 * @brief Runs before the Python interpreter is initialized.
 * 
 * Primarily used to flag add NRM files to the Python module search path on startup.
 * 
 * Takes no arguments, returns void.
 */
#define REPY_ON_PRE_INIT RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnPreInit) 

/**
 * @brief Event used for initializing Python modules from code strings on startup.
 * 
 * Runs immediately after the interpreter is initialized.
 * 
 * Takes a single int `success` argument, indicating whether the interpreter was started correctly. Return should void.
 */
#define REPY_ON_LOAD_MODULES RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnLoadModules)

/**
 * @brief Event used for compiling Python bytecode from code strings on startup.
 * 
 * Runs immediately after `REPY_ON_LOAD_MODULES`.
 * 
 * Takes a single int `success` argument, indicating whether the interpreter was started correctly. Should return void.
 */
#define REPY_ON_MAKE_GLOBAL_CACHES RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnMakeGlobalCaches)

/**
 * @brief Generic initialization event. Use for your own mod's initialization code.
 * 
 * Runs immediately after `REPY_ON_MAKE_GLOBAL_CACHES`.
 * 
 * Takes a single int `success` argument, indicating whether the interpreter was started correctly. Should return void.
 */
#define REPY_ON_INIT RECOMP_CALLBACK(REPY_MOD_ID_STR, REPY_OnInit)

/** @}*/

/** \defgroup repy_init_macros Initialization Macros
 *  @{
 */

/**
 * @brief Adds this .nrm file to Python's module search path.
 * 
 * This will allow you to add Python modules (both single files and module folders) to your mod by
 * including them under the `additional_files` section of your mod.toml
 * 
 */
#define PRE_INIT_ADD_NRM_TO_MODULE_PATH \
REPY_ON_PRE_INIT void _repy_register_nrm () { \
    const unsigned char* nrm_file_path = recomp_get_mod_file_path(); \
    REPY_PreInitAddToModuleSearchPath(nrm_file_path); \
    recomp_free((void*)nrm_file_path); \
};

#ifdef REPY_SILENCE_INCBIN_SQUIGGLES
#define REPY_INCBIN(identifier, filename)      \
    extern u8 identifier[];                    \
    extern u8 identifier##_end[]

#define REPY_INCBIN_TEXT(identifier, filename) \
    extern u8 identifier[];                    \
    extern u8 identifier##_end[]               
#else
/**
 * @brief General INCBIN macro used by several other initialization macros to include external Python code.
 * 
 * The data included by this macro is not NULL-terminated. You'll need to use `identifier_end` to find the
 * end of the code-block and use `identifier_end - identifier` to find the length.
 * 
 * @param identifier The variable name for the start of the data. `identifier_end` will indicate the end of the data.
 * @param filename The path to the included file. The file needs to be in your include path.
 */
#define REPY_INCBIN(identifier, filename)      \
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

    /**
 * @brief General INCBIN macro used by several other initialization macros to include external Python code.
 * 
 * The data included by this macro is NULL-terminated, with `identifier_end` pointing to the termination character.
 * Therefore, you'll need to use `identifier_end - identifier - 1` to find the length of the text.
 * 
 * @param identifier the variable name for the start of the data. `identifier_end` will indicate the end of the data.
 * @param filename the path to the included file. The file needs to be in your include path.
 */
#define REPY_INCBIN_TEXT(identifier, filename)      \
    asm(".pushsection .rodata\n"                      \
        "\t.globl " #identifier "\n"                  \
        "\t.type " #identifier ", @object\n"          \
        "\t.balign 8\n"                               \
        #identifier ":\n"                             \
        "\t.incbin \"" filename "\"\n"                \
        "\t.space 1"                                  \
        "\t.globl " #identifier "_end\n"              \
        #identifier "_end:\n"                         \
        "\t.popsection\n");                           \
    extern u8 identifier[];                           \
    extern u8 identifier##_end[]
#endif

/**
 * @brief On startup, construct a Python module from an INCBINed file. Use outside of any functions.
 * 
 * Note that in general, the preferred method of including Python modules in your mod is to include them in your
 * .nrm under the `additional files` section. This interface will likely be deprecated in a future update.
 * 
 * @param module_name The name for your module as it would be used in import statements. Remember that modules are shared across 
 * all mods using REPY, so be sure to make the name unique.
 * @param filename The path to the module file to INCBIN. The file needs to be in your include path. 
 */
#define REPY_INCBIN_MODULE(module_name, filename) \
REPY_INCBIN(module_name ## _code, filename); \
REPY_ON_LOAD_MODULES void _construct_module_ ## module_name (int success) { \
    if (success) { \
        REPY_LoadModuleN(#module_name, (const char*)module_name ## _code, (u32) (module_name ## _code_end - module_name ## _code)); \
    } \
}

/**
 * @brief On startup, compiles a Python code string into bytecode with a global handle. Use outside of any functions.
 * 
 * To access the the bytecode from another source file, use `extern REPY_Handle bytecode_identifier;`,
 * where `bytecode_identifier` is the variable name you entered in the `bytecode_identifier` parameter.
 * 
 * @param bytecode_identifier The variable name for the resultant `REPY_Handle` bytecode handle.
 * @param code_mode The type of code being compiled. See `REPY_CodeMode` for valid modes.
 * @param code_str The Python code string to compile. Must be NULL-terminated.
 */
#define REPY_GLOBAL_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        char* iden_str = REPY_InlineCodeSourceStrHelper("REPY_GLOBAL_COMPILE_CACHE", __FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
        bytecode_identifier = REPY_CompileCStr(code_str, (const char*)iden_str, code_mode); \
        recomp_free(iden_str); \
    } \
}

/**
 * @brief On startup, compiles a Python code string into bytecode with a static handle. Use outside of any functions.
 * 
 * Because the `REPY_Handle` variable is marked as `static`, there will be no accessing it from other source files.
 * 
 * @param bytecode_identifier The variable name for the resultant `static REPY_Handle` bytecode handle.
 * @param code_mode The type of code being compiled. See `REPY_CodeMode` for valid modes.
 * @param code_str The Python code string to compile. Must be NULL-terminated.
 */
#define REPY_STATIC_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
static REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        char* iden_str = REPY_InlineCodeSourceStrHelper("REPY_STATIC_COMPILE_CACHE", __FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
        bytecode_identifier = REPY_CompileCStr(code_str, (const char*)iden_str, code_mode); \
        recomp_free(iden_str); \
    } \
}

/**
 * @brief On startup, compiles a INCBINed Python code file into bytecode with a global handle.
 * 
 * To access the the bytecode from another source file, use `extern REPY_Handle bytecode_identifier;`,
 * where `bytecode_identifier` is the variable name you entered in the `bytecode_identifier` parameter.
 * 
 * Unlike `REPY_GLOBAL_COMPILE_CACHE`, this macro assumes a REPY_CodeMode of `REPY_CODE_EXEC`.
 * 
 * @param bytecode_identifier The variable name for the resultant `REPY_Handle` bytecode handle.
 * @param filename The path to the module file to INCBIN. The file needs to be in your include path. 
 */
#define REPY_GLOBAL_COMPILE_INCBIN_CACHE(bytecode_identifier, filename) \
REPY_INCBIN(bytecode_identifier ## _code_str, filename); \
REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        char* iden_str = REPY_InlineCodeSourceStrHelper("REPY_GLOBAL_COMPILE_INCBIN_CACHE: " filename, __FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
        bytecode_identifier = REPY_CompileCStrN(bytecode_identifier ## _code_str, bytecode_identifier ## _code_str_end - bytecode_identifier ## _code_str, \
            (const char*)iden_str, REPY_CODE_EXEC); \
        recomp_free(iden_str); \
    } \
}

/**
 * @brief On startup, compiles a INCBINed Python code file into bytecode with a global handle. Use outside of any functions.
 * 
 * Because the `REPY_Handle` variable is marked as `static`, there will be no accessing it from other source files.
 * 
 * Unlike `REPY_GLOBAL_COMPILE_CACHE`, this macro assumes a REPY_CodeMode of `REPY_CODE_EXEC`.
 * 
 * @param bytecode_identifier The variable name for the resultant `static REPY_Handle` bytecode handle.
 * @param filename The path to the module file to INCBIN. The file needs to be in your include path. 
 */
#define REPY_STATIC_COMPILE_INCBIN_CACHE(bytecode_identifier, filename) \
REPY_INCBIN(bytecode_identifier ## _code_str, filename); \
REPY_Handle bytecode_identifier = 0; \
REPY_ON_MAKE_GLOBAL_CACHES void _cache_code_ ## bytecode_identifier (int success) { \
    if (success && bytecode_identifier == 0) { \
        char* iden_str = REPY_InlineCodeSourceStrHelper("REPY_STATIC_COMPILE_INCBIN_CACHE: " filename, __FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
        bytecode_identifier = REPY_CompileCStrN(bytecode_identifier ## _code_str, bytecode_identifier ## _code_str_end - bytecode_identifier ## _code_str, \
            (const char*)iden_str, REPY_CODE_EXEC); \
        recomp_free(iden_str); \
    } \
}
/** @}*/

/** \defgroup repy_inline_cache_macros Inline Code Caching Macros
 *  @{
 */

/**
 * @brief Inside a function, construct a code block that compiles to bytecode a Python code string the first time it's run,
 * storing the handle for that bytecode in a static variable. Allows setting a category name.
 * 
 * This macro forms the backbone of many of the other macros in this header, expecially those in the `REPY_FN` section.
 * By only parsing and compiling the Python code string once, we can dramatically improve the performance of any function
 * that needs to execute inline Python code.
 * 
 * The category argument becomes part of the resultant bytecode object's `identifier` string, which is displayed in Python error messages.
 * In this header, macros that depend this one will use their own name for this argument. That way, if you get a error message from running
 * inline Python code, you can see which macro caused the error along with the other source information.
 * 
 * @param category A category name used as part of the bytecode's identifying string. 
 * @param bytecode_identifier The name of the static variable that the bytecode handle will be assigned to.
 * @param code_mode The type of code being compiled. See `REPY_CodeMode` for valid modes.
 * @param code_str The Python code string to compile. Should be NULL-terminated.
 */
#define REPY_INLINE_COMPILE_CACHE_BLOCK(category, bytecode_identifier, code_mode, code_str) \
static REPY_Handle bytecode_identifier = 0; \
if (bytecode_identifier == 0) { \
    char* iden_str = REPY_InlineCodeSourceStrHelper(category, __FILE_NAME__, (char*) __func__, __LINE__, #bytecode_identifier); \
    bytecode_identifier = REPY_CompileCStr(code_str, (const char*)iden_str, code_mode); \
    recomp_free(iden_str); \
}

/**
 * @brief Inside a function, construct a code block that compiles to bytecode a Python code string the first time it's run,
 * storing the handle for that bytecode in a static variable.
 * 
 * By only parsing and compiling the Python code string once, we can dramatically improve the performance of any function
 * that needs to execute inline Python code.
 * 
 * This macro is identical to `REPY_INLINE_COMPILE_CACHE_BLOCK`, except it uses REPY_INLINE_COMPILE_CACHE as the category.
 * 
 * @param bytecode_identifier The name of the static variable that the bytecode handle will be assigned to.
 * @param code_mode The type of code being compiled. See `REPY_CodeMode` for valid modes.
 * @param code_str The Python code string to compile. Should be NULL-terminated.
 */
#define REPY_INLINE_COMPILE_CACHE(bytecode_identifier, code_mode, code_str) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_INLINE_COMPILE_CACHE", bytecode_identifier, code_mode, code_str)

/** @}*/

/** \defgroup repy_foreach_macros Python Foreach Macros
 *  @{
 */

/**
 * @brief General macro to construct a `for` loop that uses `REPY_IteratorHelper`.
 * 
 * Serves as the basis for the `REPY_FOREACH` and `REPY_FN_FOREACH_CACHE` macros.
 * 
 * The arguments `py_scope_nullable` and `var_name` are used specifically by `REPY_FN_FOREACH_CACHE` for
 * scope management. Each call of `REPY_IteratorHelper_Update` the Python object for `iter_identifier->curr` will
 * be added to `py_scope_nullable` under a name set by the `var_name` argument. This behavior is disabled if 
 * `py_scope_nullable` is set to `REPY_NO_OBJECT`. Also, the `py_scope_nullable` handle is copied on initialization, 
 * meaning that it's safe to release the original. 
 * 
 * REPY_IteratorHelper doesn't hold onto the `py_object` handle you pass in, so it's safe to release it.
 * 
 * In this macro, `REPY_IteratorHelper_Update` is set to clean up the `REPY_IteratorHelper` automatically. once the iteration ends.
 * Ergo, you only have to clean up if you end the loop early, such as through a break or a return. The macros `REPY_FOREACH_CLEANUP_NOW`,
 * `REPY_FOREACH_BREAK`, and `REPY_FOREACH_RETURN` are provided to facillitate that. Failure to manually clean up the `REPY_IteratorHelper`
 * when exiting the loop early will result in a memory leak.
 * 
 * @param iter_identifier the variable name for the `REPY_IteratorHelper` pointer.
 * @param py_object a `REPY_Handle` for the Python object to iterate through.
 * @param py_scope_nullable a `REPY_Handle` to a Python `dict` being used a local scope. Can be `REPY_NO_OBJECT`.
 * @param var_name the variable name for the `REPY_IteratorHelper` pointer. If `py_scope_nullable` is set to `REPY_NO_OBJECT`,
 * this does nothing and should be set to NULL.
 */
#define REPY_FOREACH_BLOCK(iter_identifier, py_object, py_scope_nullable, var_name) \
for (REPY_IteratorHelper* iter_identifier = REPY_IteratorHelper_Create(py_object, py_scope_nullable, var_name); REPY_IteratorHelper_Update(iter_identifier, true);)

/**
 * @brief Iterate through a Python object, the way Python's `for` loops do.
 * 
 * A `REPY_IteratorHelper` object is created to manage the iteration process. The current object of the loop can be
 * accessed via `iter_identifier->curr`, and the index of that object can be accessed via `iter_identifier->index`.
 * See the `REPY_IteratorHelper` documentation for more information.
 * 
 * REPY_IteratorHelper doesn't hold onto the `py_object` handle you pass in, so it's safe to release it.
 * 
 * In this macro, `REPY_IteratorHelper_Update` is set to clean up the `REPY_IteratorHelper` automatically. once the iteration ends.
 * Ergo, you only have to clean up if you end the loop early, such as through a break or a return. The macros `REPY_FOREACH_CLEANUP_NOW`,
 * `REPY_FOREACH_BREAK`, and `REPY_FOREACH_RETURN` are provided to facillitate that. ailure to manually clean up the `REPY_IteratorHelper`
 * when exiting the loop early will result in a memory leak.
 * 
 * @param iter_identifier the variable name for the `REPY_IteratorHelper` pointer.
 * @param py_object a `REPY_Handle` for the Python object to iterate through.
 */
#define REPY_FOREACH(iter_identifier, py_object) \
REPY_FOREACH_BLOCK(iter_identifier, py_object, 0, NULL)

/**
 * @brief Manually clean up the `REPY_IteratorHelper` for a `REPY_FOREACH` loop.
 * 
 * Once this had been called, you'll have a crash if you attempt another pass of the loop.
 * 
 * @param iter_identifier the `REPY_IteratorHelper` pointer.
 */
#define REPY_FOREACH_CLEANUP_NOW(iter_identifier) \
REPY_IteratorHelper_Destroy(iter_identifier)

/**
 * @brief Manually clean up the `REPY_IteratorHelper` for a `REPY_FOREACH` loop, and immediately break.
 * 
 * @param iter_identifier the `REPY_IteratorHelper` pointer.
 */
#define REPY_FOREACH_BREAK(iter_identifier) \
REPY_FOREACH_CLEANUP_NOW(iter_identifier); break

/**
 * @brief Manually clean up the `REPY_IteratorHelper` for a `REPY_FOREACH` loop, and immediately return.
 * 
 * Supports returning a value.
 * 
 * @param iter_identifier the `REPY_IteratorHelper` pointer.
 */
#define REPY_FOREACH_RETURN(iter_identifier) \
REPY_FOREACH_CLEANUP_NOW(iter_identifier); return

// FN - Overhead: 
#define REPY_FN_SETUP \
REPY_Handle _py_globals = REPY_CreateEmptyDict(); \
REPY_Handle _py_locals = _py_globals \

#define REPY_FN_SETUP_WITH_GLOBALS(globals) \
REPY_Handle _py_globals = globals; \
REPY_Handle _py_locals = REPY_CreateEmptyDict() \

#define REPY_FN_SETUP_GLOBALS_ONLY(globals) \
REPY_Handle _py_globals = globals; \
REPY_Handle _py_locals = _py_globals; \

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
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EXEC_CACHE", identifier, REPY_CODE_EXEC, code_str) \
u32 identifier ## _success = REPY_FN_EXEC(identifier) 

// FN - Eval Bytecode:
#define REPY_FN_EVAL(bytecode_handle) \
REPY_Eval(bytecode_handle, _py_globals, _py_locals) 

#define REPY_FN_EVAL_BOOL(bytecode_handle) \
REPY_CastBool(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_U8(bytecode_handle) \
REPY_CastU8(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_S8(bytecode_handle) \
REPY_CastS8(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_U16(bytecode_handle) \
REPY_CastU16(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

#define REPY_FN_EVAL_S16(bytecode_handle) \
REPY_CastS16(REPY_MakeSUH(REPY_Eval(bytecode_handle, _py_globals, _py_locals)))

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

#define REPY_FN_EVAL_CSTR_U8(code_str) \
REPY_CastU8(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S8(code_str) \
REPY_CastS8(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U16(code_str) \
REPY_CastU16(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_S16(code_str) \
REPY_CastS16(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

#define REPY_FN_EVAL_CSTR_U16(code_str) \
REPY_CastU16(REPY_MakeSUH(REPY_EvalCStr(code, _py_globals, _py_locals)))

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
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE", identifier, REPY_CODE_EVAL, code_str) \
REPY_Handle out_var = REPY_FN_EVAL(identifier)

#define REPY_FN_EVAL_CACHE_BOOL(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_BOOL", identifier, REPY_CODE_EVAL, code_str) \
bool out_var = REPY_FN_EVAL_BOOL(identifier)

#define REPY_FN_EVAL_CACHE_U8(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_U8", identifier, REPY_CODE_EVAL, code_str) \
u32 out_var = REPY_FN_EVAL_U8(identifier)

#define REPY_FN_EVAL_CACHE_S8(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_S8", identifier, REPY_CODE_EVAL, code_str) \
s32 out_var = REPY_FN_EVAL_S8(identifier)

#define REPY_FN_EVAL_CACHE_U16(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_U16", identifier, REPY_CODE_EVAL, code_str) \
u32 out_var = REPY_FN_EVAL_U16(identifier)

#define REPY_FN_EVAL_CACHE_S16(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_S16", identifier, REPY_CODE_EVAL, code_str) \
s32 out_var = REPY_FN_EVAL_S16(identifier)

#define REPY_FN_EVAL_CACHE_U32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_U32", identifier, REPY_CODE_EVAL, code_str) \
u32 out_var = REPY_FN_EVAL_U32(identifier)

#define REPY_FN_EVAL_CACHE_S32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_S32", identifier, REPY_CODE_EVAL, code_str) \
s32 out_var = REPY_FN_EVAL_S32(identifier)

#define REPY_FN_EVAL_CACHE_F32(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_F32", identifier, REPY_CODE_EVAL, code_str) \
f32 out_var = REPY_FN_EVAL_F32(identifier)

#define REPY_FN_EVAL_CACHE_U64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_U64", identifier, REPY_CODE_EVAL, code_str) \
u64 out_var = REPY_FN_EVAL_U64(identifier)

#define REPY_FN_EVAL_CACHE_S64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_S64", identifier, REPY_CODE_EVAL, code_str) \
s64 out_var = REPY_FN_EVAL_S64(identifier)

#define REPY_FN_EVAL_CACHE_F64(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_F64", identifier, REPY_CODE_EVAL, code_str) \
f64 out_var = REPY_FN_EVAL_F64(identifier)

#define REPY_FN_EVAL_CACHE_STR(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_STR", identifier, REPY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_STR(identifier)

#define REPY_FN_EVAL_CACHE_BYTESTR(identifier, code_str, out_var) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_EVAL_CACHE_BYTESTR", identifier, REPY_CODE_EVAL, code_str) \
char* out_var = REPY_FN_EVAL_BYTESTR(identifier)


// Scope Management - Modules:
#define REPY_FN_IMPORT(module_name) \
REPY_DictSetCStr(_py_locals, module_name, REPY_MakeSUH(REPY_ImportModule(module_name)))

// Scope Management - Primatives:
#define REPY_FN_GET(var_name) \
REPY_DictGetCStr(_py_locals, var_name);

#define REPY_FN_SET(var_name, py_object) \
REPY_DictSetCStr(_py_locals, var_name, py_object)

#define REPY_FN_GET_BOOL(var_name) \
REPY_CastBool(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_BOOL(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBool(value)))

#define REPY_FN_GET_U8(var_name) \
REPY_CastU8(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_U8(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU8(value)))

#define REPY_FN_GET_S8(var_name) \
REPY_CastS8(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_S8(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS8(value)))

#define REPY_FN_GET_U16(var_name) \
REPY_CastU16(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_U16(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU16(value)))

#define REPY_FN_GET_S16(var_name) \
REPY_CastS16(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_S16(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS16(value)))

#define REPY_FN_GET_U32(var_name) \
REPY_CastU32(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_U32(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU32(value)))

#define REPY_FN_GET_S32(var_name) \
REPY_CastS32(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_S32(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS32(value)))

#define REPY_FN_GET_F32(var_name) \
REPY_CastF32(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_F32(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateF32(value)))

#define REPY_FN_GET_U64(var_name) \
REPY_CastU64(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_U64(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateU64(value)))

#define REPY_FN_GET_S64(var_name) \
REPY_CastS64(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_S64(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateS64(value)))

#define REPY_FN_GET_F64(var_name) \
REPY_CastF64(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_F64(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateF64(value)))

// Scope Management - Strings
#define REPY_FN_GET_STR(var_name) \
REPY_CastStr(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, var_name)))

#define REPY_FN_SET_STR(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateStr(value)))

#define REPY_FN_SET_STR_N(var_name, value, len) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateStr(value, len)))


// Scope Management - Byte Strings
#define REPY_FN_GET_BYTESTR(var_name) \
REPY_CastBytes(REPY_MakeSUH(REPY_DictGetCStr(_py_locals, REPY_MakeSUH(REPY_CreateBytes(var_name)))))

#define REPY_FN_SET_BYTESTR(var_name, value) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBytes(value)))

#define REPY_FN_SET_BYTESTR_N(var_name, value, len) \
REPY_DictSetCStr(_py_locals, var_name, REPY_MakeSUH(REPY_CreateBytes(value, len)))
// Flow Control - If:

#define REPY_FN_IF_CACHE_INIT_BLOCK(helper_identifier) \
static REPY_IfStmtChain* helper_identifier ## _chain_root = NULL; \
REPY_IfStmtHelper helper_identifier; \
REPY_IfStmtHelper_InitInPlace(&helper_identifier, &helper_identifier ## _chain_root); 

#define REPY_FN_IF_STMT_CACHE(helper_identifier, py_expression) \
if ( \
    REPY_IfStmtHelper_Step( \
        &helper_identifier, \
        _py_globals, \
        _py_locals, \
        py_expression, \
        __FILE_NAME__, \
        (char*) __func__, \
        __LINE__, \
        #helper_identifier \
    ) \
) 

#define REPY_FN_IF_CACHE(helper_identifier, py_expression) \
REPY_FN_IF_CACHE_INIT_BLOCK(helper_identifier) \
REPY_FN_IF_STMT_CACHE(helper_identifier, py_expression) 

#define REPY_FN_ELIF_CACHE(helper_identifier, py_expression) \
else REPY_FN_IF_STMT_CACHE(helper_identifier, py_expression)


// Flow Control - Loops
#define REPY_FN_WHILE_CACHE(bytecode_identifier, py_expression) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_WHILE_CACHE", bytecode_identifier, REPY_CODE_EVAL, py_expression); \
while (REPY_FN_EVAL_BOOL(bytecode_identifier))

#define REPY_FN_FOREACH_CACHE(bytecode_identifier, var_name, py_expression) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_FOREACH_CACHE", bytecode_identifier, REPY_CODE_EVAL, py_expression); \
REPY_FOREACH_BLOCK(bytecode_identifier ## _iter, REPY_MakeSUH(REPY_FN_EVAL(bytecode_identifier)), _py_locals, var_name)

#define REPY_FN_FOREACH_CACHE_CLEANUP_NOW(bytecode_identifier) \
REPY_IteratorHelper_Destroy(bytecode_identifier ## _iter)

#define REPY_FN_FOR_CACHE(bytecode_identifier, py_init_statement, py_eval_expression, py_after_statement) \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_FOR_CACHE", bytecode_identifier ## _init_statement, REPY_CODE_EXEC, py_init_statement); \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_FOR_CACHE", bytecode_identifier ## _eval_expression, REPY_CODE_EVAL, py_eval_expression); \
REPY_INLINE_COMPILE_CACHE_BLOCK("REPY_FN_FOR_CACHE", bytecode_identifier ## _after_statement, REPY_CODE_EXEC, py_after_statement); \
for ( \
    REPY_FN_EXEC(bytecode_identifier ## _init_statement); \
    REPY_FN_EVAL_BOOL(bytecode_identifier ## _eval_expression); \
    REPY_FN_EXEC(bytecode_identifier ## _after_statement) \
) 

// Preinit:
REPY_IMPORT(void REPY_PreInitAddToModuleSearchPath(const unsigned char* nrm_file_path));

// General:
REPY_IMPORT(void REPY_Release(REPY_Handle py_object));
REPY_IMPORT(REPY_Handle REPY_MakeSUH(REPY_Handle py_handle_no_release));
REPY_IMPORT(bool REPY_IsValidHandle(REPY_Handle py_handle_no_release));
REPY_IMPORT(bool REPY_GetSUH(REPY_Handle py_handle_no_release));
REPY_IMPORT(void REPY_SetSUH(REPY_Handle py_handle_no_release, bool value));
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

// Pointers:
REPY_IMPORT(REPY_Handle REPY_CreatePtr(void* value));
REPY_IMPORT(REPY_Handle REPY_CreatePtr_SUH(void* value));
REPY_IMPORT(void* REPY_CastPtr(REPY_Handle object));

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

// Iteration
REPY_IMPORT(REPY_Handle REPY_Iter(REPY_Handle object));
REPY_IMPORT(REPY_Handle REPY_Next(REPY_Handle iterator, REPY_Handle default_obj_nullable, u32 process_stop_iteration));

// Tuple:
REPY_IMPORT(REPY_Handle REPY_CreateTuple(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreateTuple_SUH(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreatePair(REPY_Handle key, REPY_Handle value));
REPY_IMPORT(REPY_Handle REPY_CreatePair_SUH(REPY_Handle key, REPY_Handle value));
REPY_IMPORT(REPY_Handle REPY_TupleGetIndexS32(REPY_Handle tuple, int index));
// Dicts:
REPY_IMPORT(REPY_Handle REPY_CreateEmptyDict());
REPY_IMPORT(REPY_Handle REPY_CreateEmptyDict_SUH());
REPY_IMPORT(REPY_Handle REPY_CreateDict(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_CreateDict_SUH(u32 size, ...));
REPY_IMPORT(REPY_Handle REPY_DictGet(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(REPY_Handle REPY_DictGetCStr(REPY_Handle dict, char* key));
REPY_IMPORT(void REPY_DictSet(REPY_Handle dict, REPY_Handle key, REPY_Handle value));
REPY_IMPORT(void REPY_DictSetCStr(REPY_Handle dict, char* key, REPY_Handle value));
REPY_IMPORT(bool REPY_DictHas(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(bool REPY_DictHasCStr(REPY_Handle dict, char* key));
REPY_IMPORT(void REPY_DictDel(REPY_Handle dict, REPY_Handle key));
REPY_IMPORT(void REPY_DictDelCStr(REPY_Handle dict, char* key));

// Object Attributes:
REPY_IMPORT(REPY_Handle REPY_GetAttr(REPY_Handle obj, REPY_Handle key, REPY_Handle default_value_nullable));
REPY_IMPORT(REPY_Handle REPY_GetAttrCStr(REPY_Handle obj, char* key, REPY_Handle default_value_nullable));
REPY_IMPORT(void REPY_SetAttr(REPY_Handle obj, REPY_Handle key, REPY_Handle value));
REPY_IMPORT(void REPY_SetAttrCStr(REPY_Handle obj, char* key, REPY_Handle value));
REPY_IMPORT(bool REPY_HasAttr(REPY_Handle obj, REPY_Handle key));
REPY_IMPORT(bool REPY_HasAttrCStr(REPY_Handle obj, char* key));
REPY_IMPORT(void REPY_DelAttr(REPY_Handle obj, REPY_Handle key));
REPY_IMPORT(void REPY_DelAttrCStr(REPY_Handle obj, char* key));

// Execution:
REPY_IMPORT(REPY_Handle REPY_Compile(REPY_Handle code, REPY_Handle identifier, REPY_Handle mode));
REPY_IMPORT(REPY_Handle REPY_CompileCStr(const char* code, const char* identifier, REPY_CodeMode mode));
REPY_IMPORT(REPY_Handle REPY_CompileCStrN(const char* code, u32 len, const char* identifier, REPY_CodeMode mode));
REPY_IMPORT(bool REPY_Exec(REPY_Handle code, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));
REPY_IMPORT(bool REPY_ExecCStr(const char* code, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));
REPY_IMPORT(bool REPY_ExecCStrN(const char* code, u32 len, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));
REPY_IMPORT(REPY_Handle REPY_Eval(REPY_Handle code, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));
REPY_IMPORT(REPY_Handle REPY_EvalCStr(const char* code, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));
REPY_IMPORT(REPY_Handle REPY_EvalCStrN(const char* code, u32 len, REPY_Handle global_scope_nullable, REPY_Handle local_scope_nullable));

// Python Function Calls
REPY_IMPORT(bool REPY_Call(REPY_Handle func, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));
REPY_IMPORT(REPY_Handle REPY_CallReturn(REPY_Handle func, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));
REPY_IMPORT(bool REPY_CallAttr(REPY_Handle func, REPY_Handle name, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));
REPY_IMPORT(bool REPY_CallAttrCStr(REPY_Handle func, char* name, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));
REPY_IMPORT(REPY_Handle REPY_CallAttrReturn(REPY_Handle func, REPY_Handle name, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));
REPY_IMPORT(REPY_Handle REPY_CallAttrCStrReturn(REPY_Handle func, char* name, REPY_Handle args_nullable, REPY_Handle kwargs_nullable));

// Error Handling
REPY_IMPORT(bool REPY_IsErrorSet());
REPY_IMPORT(REPY_Handle REPY_GetErrorType());
REPY_IMPORT(REPY_Handle REPY_GetErrorTrace());
REPY_IMPORT(REPY_Handle REPY_GetErrorValue());
REPY_IMPORT(void REPY_ClearError());

// Helpers:
REPY_IMPORT(char* REPY_InlineCodeSourceStrHelper(char* category, char* filename, char* function_name, u32 line_number, char* identifier));

REPY_IMPORT(REPY_IteratorHelper* REPY_IteratorHelper_Create(REPY_Handle py_object, REPY_Handle py_scope_nullable, const char* var_name));
REPY_IMPORT(void REPY_IteratorHelper_Destroy(REPY_IteratorHelper* helper));
REPY_IMPORT(bool REPY_IteratorHelper_Update(REPY_IteratorHelper* helper, bool auto_destroy));

REPY_IMPORT(REPY_IfStmtChain* REPY_IfStmtChain_Create(char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier));
REPY_IMPORT(void REPY_IfStmtChain_Destroy(REPY_IfStmtChain* chain));
REPY_IMPORT(void REPY_IfStmtHelper_InitInPlace(REPY_IfStmtHelper* helper, REPY_IfStmtChain** root));
REPY_IMPORT(bool REPY_IfStmtHelper_Step(REPY_IfStmtHelper* helper, REPY_Handle global_scope, REPY_Handle local_scope, char* expr_string, char* filename, char* function_name, u32 line_number, char* identifier));

#endif