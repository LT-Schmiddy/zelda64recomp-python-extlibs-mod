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
RECOMP_IMPORT(".", int PythonNative_ReleaseScope(PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_Execute(PythonBytecodeHandle handle, PythonScopeHandle scope));
RECOMP_IMPORT(".", int PythonNative_ExecuteString(const char* str, PythonScopeHandle scope));

// Patches a function in the base game that's used to check if the player should quickspin.
RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int py_init = PythonNative_Init(mod_folder);
    recomp_free((void*)mod_folder);

    if (py_init == 0) {
        recomp_printf("Python interpreter initialized successfully.\n");
    } else {
        recomp_printf("There was an error initializing the Python interpreter.\n");
    }

    PythonScopeHandle py_scope = PythonNative_CreateScope();
    PythonBytecodeHandle bc = PythonNative_CompileBytecode(
        "print(\"Hello Alex from Inline Python!\")\n"
        "import threading, time\n"
        "def test_fn():\n"
        "    time.sleep(5)\n"
        "    print('Hello from a thread.')\n"
        "\n"
        "t = threading.Thread(target=test_fn)\n"
        "t.start()\n"
        ,
        "Inline Codeblock 1",
        PY_CODE_EXEC
    );
    PythonNative_Execute(bc, py_scope);
    PythonNative_ReleaseScope(py_scope);
    PythonNative_ReleaseBytecode(bc);

}


