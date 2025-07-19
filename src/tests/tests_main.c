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

// Patches a function in the base game that's used to check if the player should quickspin.
REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");

    REPY_FN_SET_U64("u64val1", 99);
    REPY_FN_SET_S64("s64val1", -99);
    REPY_FN_SET_F64("f64val1", 99.5);
    REPY_FN_EXEC_BLOCK(
        create_int1,
        "print(f'{u64val1=}')\n"
        "print(f'{s64val1=}')\n"
        "print(f'{f64val1=}')\n"
        "u64val2 = 33\n"
        "s64val2 = -33\n"
        "f64val2 = 33.0\n"
    );

    u64 u64val2 = REPY_FN_GET_U64("u64val2");
    s64 s64val2 = REPY_FN_GET_S64("s64val2");
    f64 f64val2 = REPY_FN_GET_F64("f64val2");
    
    recomp_printf("llu1 %llu (size %u)\n", u64val2, sizeof(u64val2));
    recomp_printf("llu2 %lli (size %u)\n", s64val2, sizeof(s64val2));
    recomp_printf("llu2 %llf (size %u)\n", f64val2, sizeof(f64val2));

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}
