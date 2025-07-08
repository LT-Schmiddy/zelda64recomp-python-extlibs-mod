#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

RECOMP_IMPORT(".", int PythonNative_Init(const unsigned char* str));
RECOMP_IMPORT(".", int PythonNative_Call(const unsigned char* name, ...));

// Patches a function in the base game that's used to check if the player should quickspin.
RECOMP_CALLBACK("*", recomp_on_init) void Python_Init() {
    const unsigned char* mod_folder = recomp_get_mod_folder_path();
    int retVal = PythonNative_Init(mod_folder);
    recomp_free((void*)mod_folder);

    if (retVal == 0) {
        recomp_printf("Python interpreter initialized successfully.\n");
    } else {
        recomp_printf("There was an error initializing the Python interpreter.\n");
    }
}


