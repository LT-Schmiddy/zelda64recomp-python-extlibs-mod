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

    u8 bytes_array[10] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
    PyObjectHandle bytes_test = REPY_MemcpyToByteArray(bytes_array, 10, false);
    REPY_FN_SET("bytes_test", bytes_test);

    REPY_FN_EXEC_BLOCK(
        memcpy_test1,
        "print(f'{len(bytes_test)=}')\n"
        "print(f'{bytes_test=}')\n"
        "for i in range(0, len(bytes_test)):\n"
        "    bytes_test[i] += 1\n"
        "\n"
    );
    REPY_MemcpyFromByteArray(bytes_array, 10, false, bytes_test);
    recomp_printf("BytesArray: ");
        
    for (int i = 0; i < 10; i++) {
        recomp_printf("%i", bytes_array[i]);
    }
    recomp_printf("\n");

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}