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
    
    recomp_printf("Case %s %s\n", case_name, case_stmt ? "Passed" : "Failed!");
}

REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");

    REPY_FN_SET_U8("u8_set_test", 55);
    REPY_FN_SET_S8("s8_set_test", -55);
    REPY_FN_SET_U16("u16_set_test", 55);
    REPY_FN_SET_S16("s16_set_test", -55);

    REPY_FN_EXEC_BLOCK(
        small_val_test1,
        "print(f'{u8_set_test=}')\n"
        "print(f'{s8_set_test=}')\n"
        "print(f'{u16_set_test=}')\n"
        "print(f'{s16_set_test=}')\n"
    );


    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}