#include "modding.h"
#include "global.h"
#include "recomputils.h"

#include "repy_api.h"

int _test_cases = 0;
int _test_cases_passed = 0; 

void validate(char* case_name, bool case_stmt) {
    _test_cases++;

    if (case_stmt) {
        _test_cases_passed++;
    }

    recomp_printf("Case %s %s\n", case_name, case_stmt ? "Passed" : "Failed!");
}

void inline_test() {
    // REPY_FN_SETUP;
    // REPY_FN_EXEC(
    //     block1, 
    //     "print('Hello Alex')"
    // );

    // static PyObjectHandle identifier_bytecode = 0; \
    // if (identifier_bytecode == 0) { \
    //     identifier_bytecode = REPY_CompileCStr("print('Hello Alex')", __FILE_NAME__ ", in " __FUNCTION__  ", " "identifier", PY_CODE_EXEC); \
    // }
    // u32 identifier_success = REPY_Exec(identifier_bytecode, _py_globals, _py_locals);

    // REPY_FN_RETURN;
}

// Patches a function in the base game that's used to check if the player should quickspin.
REPY_ON_INIT void REPY_Tests() {
    recomp_printf("REPY Tests Loaded\n");
    inline_test();


    recomp_printf("Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
}

