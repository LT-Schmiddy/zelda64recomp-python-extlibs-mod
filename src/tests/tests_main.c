#include "tests_main.h"
#include "tests_api.h"
#include "tests_fn_macros.h"

REPY_PREINIT_ADD_NRM_TO_SYS_PATH;

int _test_cases = 0;
int _test_cases_passed = 0; 

void validate(char* case_name, bool case_stmt) {
    _test_cases++;

    if (case_stmt) {
        _test_cases_passed++;
    }
    
    if ((!case_stmt) || recomp_get_config_u32("print_success_cases")) {
        recomp_printf("Case %s %s\n", case_stmt ? "Passed:" : "Failed!", case_name);
    }
}

REPY_REGISTER_SUBINTERPRETER(test_subinterp);


void load_repl() {
        recomp_printf("Starting Interactive Shell. Call `exit()` to continue to game...\n");
        char mem_test[] = "memoryview_test";

        REPY_Handle nrm_zip = REPY_GetNrmZipFile();
        REPY_Handle code_module = REPY_ImportModule("code");

        REPY_Handle local = REPY_CreateDict(0);
        REPY_DictSetCStr(local, "nrm_zip", nrm_zip);
        REPY_DictSetCStr(local, "mem_ptr", REPY_CreatePtr_SUH(mem_test));
        
        REPY_Handle kwargs = REPY_CreateDict(0);
        REPY_DictSetCStr(kwargs, "local", local);
        REPY_Release(local);
        
        REPY_CallAttrCStr(code_module, "interact", 0, kwargs);
        REPY_Release(kwargs);

        REPY_ClearError();
        REPY_Release(code_module);
        REPY_Release(nrm_zip);
}

REPY_ON_POST_INIT void REPY_API_Tests() {
    // Testing Interpreter Operations
    REPY_PushInterpreter(REPY_MAIN_INTERPRETER);
    validate("Interpreter == 0 after pushing main interpreter.", REPY_GetCurrentInterpreter() == 0);
    REPY_PushInterpreter(test_subinterp);
    validate("Interpreter == test_subinterp after pushing test_subinterp.", REPY_GetCurrentInterpreter() == test_subinterp);
    REPY_PopInterpreter();
    validate("Interpreter == 0 after popping test_subinterp.", REPY_GetCurrentInterpreter() == 0);

    bool test_subinterp_old_autodisarm = REPY_GetInterpreterAutoDisarm(test_subinterp);
    REPY_SetInterpreterAutoDisarm(test_subinterp, true);
    validate("Interpreter 1 Auto Disarm settable", REPY_GetInterpreterAutoDisarm(test_subinterp) && !test_subinterp_old_autodisarm);

    // Running Core API tests on interpreter 0:
    run_api_tests();
    run_main_fn_macro_tests();
    
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    {
        REPY_FN_SETUP_INTERP(test_subinterp);
        REPY_FN_EXEC_CACHE(thread_test_1, 
            "import threading, time\n"
            "def t_test():\n"
            "    time.sleep(10)\n"
            "    print('I am incredibly smart')\n"
            "\n"
            "t = threading.Thread(None, t_test)\n"
            "t.start()\n"
        );
        REPY_FN_CLEANUP;
    }

    if (recomp_get_config_u32("save_case_count")) {
        REPY_FN_SETUP;
        REPY_FN_SET_S32("test_cases", _test_cases);
        REPY_FN_SET_S32("test_cases_passed", _test_cases_passed);
        REPY_FN_EXEC_CACHE(save_count1,
            "from pathlib import Path\n"
            "Path('./test_results.txt').write_text(f'Passed {test_cases_passed} of {test_cases} test cases.')\n"
        );
        REPY_FN_CLEANUP;
    }

    if (recomp_get_config_u32("load_repl")) {
        REPY_PushInterpreter(test_subinterp);
        load_repl();
        REPY_PopInterpreter();
    }
    REPY_PopInterpreter();
}

