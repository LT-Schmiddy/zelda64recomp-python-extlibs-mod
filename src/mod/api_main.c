#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

typedef int PythonBytecodeHandle;
typedef int PythonScopeHandle;

typedef enum PythonCodeType {
    PY_CODE_EXEC = 0,
    PY_CODE_EVAL = 1,
    PY_CODE_SINGLE = 2
} PythonCodeType;

RECOMP_IMPORT(".", int PythonNative_Init(const unsigned char* str));
RECOMP_IMPORT(".", PythonBytecodeHandle PythonNative_CompileBytecode(const char* str, const char* identifier, PythonCodeType type));
RECOMP_IMPORT(".", void PythonNative_ReleaseBytecode(PythonBytecodeHandle bytecode));
RECOMP_IMPORT(".", PythonScopeHandle PythonNative_CreateScope());
RECOMP_IMPORT(".", void PythonNative_ReleaseScope(PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_Execute(PythonBytecodeHandle handle, PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_ExecuteString(const char* str, PythonScopeHandle scope));

RECOMP_IMPORT(".", u32 PythonNative_Scope_GetU32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetU32(u32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", s32 PythonNative_Scope_GetS32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetS32(s32 value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", f32 PythonNative_Scope_GetF32(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", void PythonNative_Scope_SetF32(f32 value, PythonScopeHandle scope, const char* name));

RECOMP_IMPORT(".", void PythonNative_Scope_SetString(char* value, PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetString_Prepare(PythonScopeHandle scope, const char* name));
RECOMP_IMPORT(".", int PythonNative_Scope_GetString_Copy(PythonScopeHandle scope, const char* name));

char* PythonScope_GetString(PythonScopeHandle scope, const char* name) {
    u32 len = PythonNative_Scope_GetString_Prepare(scope, name);
    char* out_str = recomp_alloc(len + 1);
    out_str[len] = 0;
    PythonNative_Scope_GetString_Copy(len, out_str);
    return out_str;
}


// Patches a function in the base game that's used to check if the player should quickspin.
RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int py_init = PythonNative_Init(mod_folder);
    recomp_free((void*)mod_folder);

    if (py_init == 1) {
        recomp_printf("Python interpreter initialized successfully.\n");
    } else {
        recomp_printf("There was an error initializing the Python interpreter.\n");
    }

    PythonScopeHandle py_scope = PythonNative_CreateScope();
    PythonNative_Scope_SetU32(22, py_scope, "uint_in");
    PythonNative_Scope_SetS32(-5, py_scope, "int_in");
    PythonNative_Scope_SetF32(10.2, py_scope, "float_in");
    PythonNative_Scope_SetString("Hello from C String", py_scope, "str_in");

    recomp_printf("Running py code.");
    PythonBytecodeHandle bc = PythonNative_CompileBytecode(
        "print(\"Hello Alex from Inline Python!\")\n"
        "print(f'{uint_in=}, {int_in=}, {float_in=}, {str_in=}')\n"
        "int_val = 10\n"
        "float_val = 5.5\n"
        "str_val = f'Hello Py String'\n"
        "\n"
        ,
        "Inline Codeblock 1",
        PY_CODE_EXEC
    );

    PythonNative_Execute(bc, py_scope);
    u32 uint_val = PythonNative_Scope_GetU32(py_scope, "int_val");
    s32 int_val = PythonNative_Scope_GetS32(py_scope, "int_val");
    f32 float_val = PythonNative_Scope_GetF32(py_scope, "float_val");
    char* str_val = PythonScope_GetString(py_scope, "str_val");

    recomp_printf("Returned values: %u, %i, %f, %s\n", uint_val, int_val, float_val, str_val);

    recomp_free(str_val);

    PythonNative_ReleaseScope(py_scope);
    PythonNative_ReleaseBytecode(bc);

}


