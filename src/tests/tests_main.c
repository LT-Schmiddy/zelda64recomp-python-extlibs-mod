#include "modding.h"
#include "global.h"
#include "recomputils.h"

#include "repy_api.h"

REPY_INCBIN_MODULE(test_module, "test_module.py");


int _test_cases = 0;
int _test_cases_passed = 0; 

void validate(char* case_name, bool case_stmt) {
    _test_cases++;

    if (case_stmt) {
        _test_cases_passed++;
    }
    
    recomp_printf("Case %s %s\n", case_stmt ? "Passed:" : "Failed!", case_name);
}

#define FIB_TABLE_SIZE 100000

REPY_ON_INIT void REPY_Tests() {
    // Testing Handle Operations:
    REPY_Handle testbool = REPY_CreateBool(true);
    validate("First assigned handle (testbool) == 1", testbool == 1);
    validate("testbool is valid (REPY_IsValidHandle)", REPY_IsValidHandle(testbool));
    validate("testbool is not SUH (REPY_GetSUH)", REPY_GetSUH(testbool) == 0);
    REPY_MakeSUH(testbool);
    validate("testbool is made SUH (REPY_MakeSUH)", REPY_GetSUH(testbool) == 1);
    REPY_SetSUH(testbool, false);
    validate("testbool SUH disabled again (REPY_MakeSUH)", REPY_GetSUH(testbool) == 0);
    REPY_Handle testbool2 = REPY_CopyHandle(testbool);
    validate("Copied handle (testbool2) == 2", testbool2 == 2);
    REPY_MakeSUH(testbool2);
    REPY_Handle testbool3 = REPY_CopyHandle(testbool2);
    validate("testbool2 is not valid after SUH access", !REPY_IsValidHandle(testbool2));
    REPY_Release(testbool);
    validate("testbool is not valid after release (REPY_Release)", !REPY_IsValidHandle(testbool));
    REPY_Release(testbool3);

    // Evaluation Tests. Doing these now since we'll need them later:
    
}