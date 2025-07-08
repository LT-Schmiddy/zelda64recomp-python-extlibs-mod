#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

RECOMP_IMPORT(".", int PythonNative_Init(const char* str));

// Patches a function in the base game that's used to check if the player should quickspin.
RECOMP_PATCH s32 Player_CanSpinAttack(Player* this) {

    int retVal = PythonNative_Init("String from mod code!");

    recomp_printf("'native_lib_test' returned %i\n", retVal);

    // Always spin attack.
    return true;
}


