#include "tests_fn_macros.h"
#include "tests_main.h"

void run_main_fn_macro_tests() {
    // These tests run after all of the API functions have had their own tests run.
    // Ergo, we will assume those work properly. These tests are purely to make sure
    // there aren't mistakes in the REPY_FN macros themselves.
    REPY_FN_SETUP;
    // First thing to test is probably the exec macros. There's only a few, and that's the easiest way 
    // to manipulate the scope without testing the get/set stuff first.
    
    // We'll start with a simple print statement:
    REPY_Handle exec_test_handle1 = REPY_CompileCStr("print('Hello from REPY_FN')", "REPY_FN_EXEC Compiled Test", REPY_CODE_EXEC);
    REPY_Handle bad_exec_test_handle1 = REPY_CompileCStr("pront('Hello from REPY_FN')", "REPY_FN_EXEC Compiled Test", REPY_CODE_EXEC);
    bool exec_success1 = REPY_FN_EXEC(exec_test_handle1);
    validate("REPY_FN_EXEC successfully executed correct bytecode", exec_success1);
    exec_success1 = REPY_FN_EXEC(bad_exec_test_handle1);
    validate("REPY_FN_EXEC failed to execute badly formed bytecode", !exec_success1);
    REPY_ClearError();

    // Now for CStr execution
    exec_success1 = REPY_FN_EXEC_CSTR("print('Hello from REPY_FN')");
    validate("REPY_FN_EXEC successfully executed correct code string", exec_success1);
    exec_success1 = REPY_FN_EXEC_CSTR("pront('Hello from REPY_FN')");
    validate("REPY_FN_EXEC failed to execute incorrect code string", !exec_success1);
    REPY_ClearError();

    // Now for cached execution:
    REPY_FN_EXEC_CACHE(exec_cache_test1,
        "print('Hello from REPY_FN')"
    );
    validate("REPY_FN_EXEC_CACHE successfully executed correct code string", exec_cache_test1_success);
    REPY_FN_EXEC_CACHE(exec_cache_test2,
        "print('Hello from REPY_FN')"
    );
    validate("REPY_FN_EXEC_CACHE failed to execute incorrect code string", !exec_cache_test2_success);

    REPY_FN_CLEANUP;
}