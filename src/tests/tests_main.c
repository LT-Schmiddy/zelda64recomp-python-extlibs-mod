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

#define FIB_TABLE_SIZE 100000

REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");
        REPY_FN_EXEC_BLOCK(
        time_start1,
        "import time\n"
        "nc_start_time = time.time()\n"
    );

    REPY_FN_EXEC_BLOCK(
        bad_fib1,
        "count_holder = 0\n"
        "def count_step():\n"
        "    global count_holder\n"
        "    count_holder += 1\n"
        "\n"
    );
    
    u64* fib_table = recomp_alloc(sizeof(u64)* FIB_TABLE_SIZE);
    PyObjectHandle fib_fn = REPY_FN_GET("count_step");

    for (int i = 0; i < FIB_TABLE_SIZE; i++) {
        REPY_Call(fib_fn, 0, 0);
        fib_table[i] = REPY_FN_GET_U64("count_holder");
        if (i % (FIB_TABLE_SIZE / 10) == 0) {
            recomp_printf("=");
        }
    }
    
    REPY_FN_EXEC_BLOCK(
        time_end1,
        "fib_run_time = time.time() - nc_start_time\n"
    );
    recomp_printf("\n... ");
    for (int i = 0; i < 5; i++) {
        recomp_printf("%llu, ", fib_table[FIB_TABLE_SIZE - 6 + i]);
    }

    REPY_FN_EXEC_BLOCK(
        time_print1,
        "print(f'{fib_run_time=}')\n"
    );
    recomp_free(fib_table);

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}