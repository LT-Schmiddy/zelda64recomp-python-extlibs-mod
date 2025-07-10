#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"

#include "extlib_functions.h"

RECOMP_INCBIN(text_module_code, "test_module.py");

char* PythonScope_GetString(PythonScopeHandle scope, const char* name) {
    u32 len = PythonNative_Scope_GetString_Prepare(scope, name);
    char* out_str = recomp_alloc(len + 1);
    out_str[len] = 0;
    PythonNative_Scope_GetString_Copy(len, out_str);
    return out_str;
}

char* PythonScope_GetBytes(PythonScopeHandle scope, const char* name) {
    u32 len = PythonNative_Scope_GetBytes_Prepare(scope, name);
    char* out_str = recomp_alloc(len + 1);
    out_str[len] = 0;
    PythonNative_Scope_GetBytes_Copy(len, out_str);
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

    PythonNative_LoadModuleN("test_module", (const char*)text_module_code, text_module_code_end - text_module_code);

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
        "bytes_val = b'Hello Py Bytes'\n"
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
    char* bytes_val = PythonScope_GetBytes(py_scope, "bytes_val");

    recomp_printf("Returned values: %u, %i, %f, %s, %s\n", uint_val, int_val, float_val, str_val, bytes_val);

    recomp_free(str_val);

    PythonNative_ReleaseScope(py_scope);
    PythonNative_ReleaseBytecode(bc);

}


