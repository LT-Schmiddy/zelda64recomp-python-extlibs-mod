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

}


