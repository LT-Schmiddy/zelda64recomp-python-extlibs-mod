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
    REPY_FN_SETUP;
    REPY_FN_IMPORT("os");

    REPY_FN_SET_S32("count", 55);

    PyObjectHandle new_dict = REPY_CreateDict();
    REPY_DictSet(new_dict, REPY_MakeSUH(REPY_CreateStr("key_string")), REPY_MakeSUH(REPY_CreateStr("value_string")));
    REPY_FN_SET("new_dict", new_dict);
    REPY_Release(new_dict);

    REPY_FN_EXEC(
        inline_test_exec1, 
        "print(f'Hello Mr. {count=}')\n"
        "print(f'Hello Mr. {new_dict=}')\n"
        
        "for i in os.listdir():\n"
        "    print('*', i)\n"
    );

    PyObjectHandle os_handle = REPY_FN_GET("os");
    const char* os_name = REPY_CastStr(REPY_MakeSUH(REPY_CallAttrReturn(os_handle, "getcwd", 0, 0)));
    recomp_printf("Python says the CWD is '%s'\n", os_name);

    REPY_Release(os_handle);
    recomp_free((void*)os_name);

    REPY_FN_RETURN;
}

void file_access_test() {
    REPY_FN_SETUP;
    REPY_FN_EXEC(
        file_access_test_exec1, 
        "from pathlib import Path\n"
        "sound_json_str = Path('sound.json').read_text()\n"
    );

    char* sound_json_str = REPY_FN_GET_STR("sound_json_str");
    recomp_printf("Sound.json Content:\n%s\n", sound_json_str);
    recomp_free(sound_json_str);

    REPY_FN_RETURN;
}

void tuple_test() {
    REPY_FN_SETUP;

    REPY_FN_EXEC(
        tuple_test_exec1, 
        "print(0, 1, 2)\n"
    );

    REPY_FN_EVAL(tuple_test_get_print, "print", print_fn);
    PyObjectHandle args = REPY_CreateTuple(3, REPY_MakeSUH(REPY_CreateS32(0)), REPY_MakeSUH(REPY_CreateS32(1)), REPY_MakeSUH(REPY_CreateS32(2)));
    REPY_FN_SET("index_2", REPY_MakeSUH(REPY_TupleGetMember(args, 2)));
    REPY_FN_EXEC(
        tuple_test_exec2, 
        "print(f'{index_2=}')\n"
    );

    REPY_Call(print_fn, args, 0);
    REPY_Release(args);
    REPY_Release(print_fn);
    REPY_FN_RETURN;
}

// Patches a function in the base game that's used to check if the player should quickspin.
REPY_ON_INIT void REPY_Tests() {
    recomp_printf("REPY Tests Loaded\n");
    // inline_test();
    // file_access_test();
    tuple_test();
    recomp_printf("Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
}

