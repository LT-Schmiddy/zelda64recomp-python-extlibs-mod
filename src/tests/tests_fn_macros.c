#include "tests_fn_macros.h"
#include "tests_main.h"


#define PY_NUM_VAL "num_val"
#define PY_STR_VAL "str_val"
#define NAMETYPE_N() name_type ## _N

// GSE stands for Get, Set, Eval
#define REPY_FN_GSE_NUM_TESTS(ctype, name_type, c_value, py_type, py_value) \
    REPY_FN_SET_ ## name_type(PY_NUM_VAL, c_value); \
    validate( \
        "REPY_FN_SET_" #name_type " set " PY_NUM_VAL, \
        REPY_DictHasCStr(REPY_FN_LOCAL_SCOPE, PY_NUM_VAL) \
    ); \
    validate("Python " PY_NUM_VAL " type is " py_type, REPY_FN_EVAL_CSTR_BOOL("type(" PY_NUM_VAL ") == " py_type)); \
    validate("Python " PY_NUM_VAL " value is " py_value, REPY_FN_EVAL_CSTR_BOOL("" PY_NUM_VAL " == " py_value)); \
    \
    ctype get_ ## ctype = REPY_FN_GET_ ## name_type(PY_NUM_VAL); \
    validate( \
        "REPY_FN_GET_" #name_type " returned " PY_NUM_VAL " as " #ctype " of " #c_value, \
        c_value == get_ ## ctype \
    ); \
    validate( \
        "REPY_FN_EVAL_" #name_type " returned " PY_NUM_VAL " as " #ctype " of " #c_value, \
        c_value == REPY_FN_EVAL_ ## name_type(REPY_CreateStr_SUH(PY_NUM_VAL)) \
    ); \
    validate( \
        "REPY_FN_EVAL_CSTR_" #name_type " returned " PY_NUM_VAL " as " #ctype " of " #c_value, \
        c_value == REPY_FN_EVAL_CSTR_ ## name_type(PY_NUM_VAL) \
    ); \
    REPY_FN_EVAL_CACHE_ ## name_type(eval_cache_bytecode_ ## ctype, PY_NUM_VAL, eval_cache_outvar_ ## ctype); \
    validate( \
        "REPY_FN_EVAL_CACHE_" #name_type " returned " PY_NUM_VAL " as " #ctype " of " #c_value, \
        c_value == eval_cache_outvar_ ## ctype \
    ); \

#define REPY_FN_GSE_STR_TESTS(ctype, name_type, c_value, len, py_type, py_value, py_byte_prefix) \
    REPY_FN_SET_ ## name_type(PY_STR_VAL, c_value); \
    validate( \
        "REPY_FN_SET_" #name_type " set " PY_STR_VAL, \
        REPY_DictHasCStr(REPY_FN_LOCAL_SCOPE, PY_STR_VAL) \
    ); \
    validate("Python " PY_STR_VAL " type is " py_type, REPY_FN_EVAL_CSTR_BOOL("type(" PY_STR_VAL ") == " py_type "")); \
    validate("Python " PY_STR_VAL " value is " py_value, REPY_FN_EVAL_CSTR_BOOL("" PY_STR_VAL " == " py_byte_prefix "\"" py_value "\"")); \
    \
    ctype* get_ ## name_type = REPY_FN_GET_ ## name_type(PY_STR_VAL); \
    validate( \
        "REPY_FN_GET_" #name_type " returned " PY_STR_VAL " as " #name_type " of " #c_value, \
        strcmp(c_value, get_ ## name_type) == 0 \
    ); \
    recomp_free(get_ ## name_type); \
    ctype* eval_ ## name_type = REPY_FN_EVAL_ ## name_type(REPY_CreateStr_SUH(PY_STR_VAL)); \
    validate( \
        "REPY_FN_EVAL_" #name_type " returned " PY_STR_VAL " as " #name_type " of " #c_value, \
        strcmp(c_value, eval_ ## name_type) == 0 \
    ); \
    recomp_free(eval_ ## name_type); \
    ctype* eval_cstr_ ## name_type = REPY_FN_EVAL_CSTR_ ## name_type(PY_STR_VAL); \
    validate( \
        "REPY_FN_EVAL_CSTR_" #name_type " returned " PY_STR_VAL " as " #name_type " of " #c_value, \
        strcmp(c_value, eval_cstr_ ## name_type) == 0 \
    ); \
    REPY_FN_EVAL_CACHE_ ## name_type(eval_cache_bytecode_ ## name_type, PY_STR_VAL, eval_cache_outvar_ ## name_type); \
    validate( \
        "REPY_FN_EVAL_CACHE_" #name_type " returned " PY_STR_VAL " as " #name_type " of " #c_value, \
         strcmp(c_value, eval_cache_outvar_ ## name_type) == 0 \
    ); \
    recomp_free(eval_cache_outvar_ ## name_type); \


void run_main_fn_macro_tests() {
    // These tests run after all of the API functions have had their own tests run.
    // Ergo, we will assume those work properly. These tests are purely to make sure
    // there aren't mistakes in the REPY_FN macros themselves.
    
    // To that end, more than needing to pass all test cases, all of these tests need to compile without
    // errors or warnings.

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
        "pront('Hello from REPY_FN')"
    );
    validate("REPY_FN_EXEC_CACHE failed to execute incorrect code string", !exec_cache_test2_success);
    // From here, assume execution works.

    // A quick sanity check on REPY_FN_EVAL_CSTR_BOOL. Knowing it works will make writing the numerical test macros easier.
    validate("REPY_FN_EVAL_CSTR_BOOL sanity check. Returns True and False correctly", REPY_FN_EVAL_CSTR_BOOL("True") && !REPY_FN_EVAL_CSTR_BOOL("False"));
    // From here, assume REPY_FN_EVAL_CSTR_BOOL works.

    // Get/Set/Eval is honestly pretty repetitive. Streamline the numerical checks with a macro:
    REPY_FN_GSE_NUM_TESTS(bool, BOOL, true, "bool", "True");
    REPY_FN_GSE_NUM_TESTS(u8, U8, 5, "int", "5");
    REPY_FN_GSE_NUM_TESTS(u16, U16, 20000, "int", "20000");
    REPY_FN_GSE_NUM_TESTS(u32, U32, 70000, "int", "70000");
    REPY_FN_GSE_NUM_TESTS(u64, U64, 5000000, "int", "5000000");
    REPY_FN_GSE_NUM_TESTS(s8, S8, -5, "int", "-5");
    REPY_FN_GSE_NUM_TESTS(s16, S16, -20000, "int", "-20000");
    REPY_FN_GSE_NUM_TESTS(s32, S32, -70000, "int", "-70000");
    REPY_FN_GSE_NUM_TESTS(s64, S64, -5000000, "int", "-5000000");
    REPY_FN_GSE_NUM_TESTS(f32, F32, -7000.5f, "float", "-7000.5");
    REPY_FN_GSE_NUM_TESTS(f64, F64, 500000.3, "float", "500000.3");
    REPY_FN_GSE_STR_TESTS(char, STR, "Hello World", 12, "str", "Hello World", "");
    REPY_FN_GSE_STR_TESTS(char, BYTESTR, "Hello World", 12, "bytes", "Hello World", "b");

    // Testing the _N setters for strings isn't easy with the naming scheme. I'll do that seperately.
    REPY_FN_SET_STR_N(PY_STR_VAL, "Hello World", 12);
    validate(
        "REPY_FN_SET_STR_N set " PY_STR_VAL,
        REPY_DictHasCStr(REPY_FN_LOCAL_SCOPE, PY_STR_VAL)
    );
    validate("Python " PY_STR_VAL " type is str", REPY_FN_EVAL_CSTR_BOOL("type(" PY_STR_VAL ") == str"));
    validate("Python " PY_STR_VAL " value is \"Hello World\"", REPY_FN_EVAL_CSTR_BOOL("" PY_STR_VAL " == \"Hello World\""));

    REPY_FN_SET_BYTESTR_N(PY_STR_VAL, "Hello World", 12);
    validate(
        "REPY_FN_SET_BYTESTR_N set " PY_STR_VAL,
        REPY_DictHasCStr(REPY_FN_LOCAL_SCOPE, PY_STR_VAL)
    );
    validate("Python " PY_STR_VAL " type is bytes", REPY_FN_EVAL_CSTR_BOOL("type(" PY_STR_VAL ") == bytes"));
    validate("Python " PY_STR_VAL " value is b\"Hello World\"", REPY_FN_EVAL_CSTR_BOOL("" PY_STR_VAL " == b\"Hello World\""));
    // From here assume all get/set/eval macros that cast work.

    
    // Testing if importing works, as well as the standard get/set/eval.
    REPY_FN_IMPORT("os");
    validate("os module was imported", REPY_DictHasCStr(REPY_FN_LOCAL_SCOPE, "os"));
    REPY_Handle get_os_module = REPY_FN_GET("os");
    REPY_FN_SET("os2", get_os_module);
    validate("os module alias created via REPY_FN_GET/SET", REPY_FN_EVAL_CSTR_BOOL("os2 is os"));
    // Ok, this handle is the real os module. Let's use that.
    REPY_Handle os_eval = REPY_FN_EVAL(REPY_CreateStr_SUH("os"));
    REPY_Handle os_eval_cstr = REPY_FN_EVAL_CSTR("os");
    REPY_FN_EVAL_CACHE(os_eval_cache_identifier, "os", os_eval_cache);

    validate("REPY_FN_EVAL returned the os module", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("_0 is _1", REPY_VL_SUH(REPY_NO_OBJECT, 2, get_os_module, os_eval), REPY_NO_OBJECT))));
    validate("REPY_FN_EVAL_CSTR returned the os module", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("_0 is _1", REPY_VL_SUH(REPY_NO_OBJECT, 2, get_os_module, os_eval_cstr), REPY_NO_OBJECT))));
    validate("REPY_FN_EVAL_CACHE returned the os module", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("_0 is _1", REPY_VL_SUH(REPY_NO_OBJECT, 2, get_os_module, os_eval_cache), REPY_NO_OBJECT))));
    // From here, assume that all get/set/eval works.

    // Now the fun part: The Pythonic control structures:
    // Lets start with for and while, since they're the simplest.
    REPY_FN_EXEC_CACHE(while_test1_setup,
        "x = 5"
    );
    int wtf = 0;
    REPY_FN_WHILE_CACHE(while_test1, "x > 0") {
        REPY_FN_EXEC_CACHE(while_test1_body,
            "x -= 1"
        );
        wtf++;
    }
    validate("REPY_FN_WHILE_CACHE while_test1 ran to completion correctly", REPY_FN_EVAL_CSTR_BOOL("x == 0") && wtf == 5);

    // Regular for loops:
    REPY_FN_FOR_CACHE(for_test1, "i = 0", "i < 10", "i += 1") {
        REPY_FN_EXEC_CACHE(for_test1_body,
            "x -= 1"
        );
        wtf++;
    }
    validate("REPY_FN_FOR_CACHE for_test1 ran to completion correctly", REPY_FN_EVAL_CSTR_BOOL("x == -10 and i == 10") && wtf == 15 );

    u32 foreach_loop_vals[5];
    bool foreach_index_check = true;
    bool foreach_handle_check = true;

    // Now for the scary part: The REPY_FN_FOREACH_CACHE loop:
    REPY_FN_FOREACH_CACHE(foreach_test1, "i", "[0, 1, 2, 3, 4, 5]") {
        u32 index = REPY_IteratorHelper_GetIndex(foreach_test1_iter);
        u32 current_val = REPY_FN_GET_U32("i");
        foreach_index_check = foreach_index_check && (index == current_val);
        
        // This handle is managed internally by the iterator helper. Do not cleanup.
        REPY_Handle current_handle = REPY_IteratorHelper_BorrowCurrent(foreach_test1_iter);
        foreach_handle_check = foreach_handle_check && (current_val == REPY_CastU32(current_handle));
        foreach_loop_vals[index] = current_val;
        
    }
    validate("REPY_FN_FOREACH_CACHE foreach_test1 rano to completion correctly.", foreach_index_check && foreach_handle_check);

    // Time to test the if statement helpers.
    // lets make some setup.
    REPY_FN_EXEC_CACHE(if_test1_prep,
        "a = 5\n"
        "b = 6\n"
        "c = 7\n"
        "d = 8\n"
    );
    bool if_test_tracking1[5];
    for (int i = 0; i < 5; i++) {
        if_test_tracking1[i] = false;
    }

    REPY_FN_IF_CACHE(if_test1, "a == 5") {
        if_test_tracking1[0] = true;
    }
    
    REPY_FN_IF_CACHE(if_test2, "b == 5") {
        if_test_tracking1[1] = true; // Shouldn't run
    }
    REPY_FN_ELIF_CACHE(if_test2, "b == 6") {
        if_test_tracking1[2] = true;
    }

    REPY_FN_IF_CACHE(if_test3, "c == 7") {
        if_test_tracking1[3] = true;
    }
    REPY_FN_ELIF_CACHE(if_test3, "d == 8") {
        if_test_tracking1[4] = true; // Shouldn't run
    }

    validate("REPY_IfStmtHelper macros behaving as expected when flat",  
        if_test_tracking1[0]
        && !if_test_tracking1[1]
        && if_test_tracking1[2]
        && if_test_tracking1[3]
        && !if_test_tracking1[4]
    );

    // Make sure these behave correctly when nested:
    bool if_test_tracking2[4];
    for (int i = 0; i < 4; i++) {
        if_test_tracking2[i] = false;
    }

    REPY_FN_IF_CACHE(if_nest1_test1, "False") {
        REPY_FN_IF_CACHE(if_nest2_test1, "True") {
            if_test_tracking2[0] = true;
        } 
        REPY_FN_ELIF_CACHE(if_nest2_test1, "False") {
            if_test_tracking2[1] = true;
        } 
    }
    REPY_FN_ELIF_CACHE(if_nest1_test1, "True") {
        REPY_FN_IF_CACHE(if_nest2_test1, "False") {
            if_test_tracking2[2] = true;
        } 
        REPY_FN_ELIF_CACHE(if_nest2_test1, "True") {
            if_test_tracking2[3] = true; // Only this should run
        } 
    }

    validate("REPY_IfStmtHelper macros behaving as expected when nested",  
        !if_test_tracking2[0]
        && !if_test_tracking2[1]
        && !if_test_tracking2[2]
        && if_test_tracking2[3]
    );

    // Testing the Foreach macros. Not technically part of `REPY_FN`, but this is still the most logical place to do this.
    REPY_FN_EVAL_CACHE(eval_list_handle1, "[5, 4, 3, 2, 1, 0]", list_handle1);
    // The behavior of the underlying IteratorHelper has already been tested.
    // Here, we just need to ensure that the macro syntax is correct.
    REPY_Handle list_borrowed1 = REPY_NO_OBJECT;
    REPY_FOREACH(eval_list_iter1, list_handle1, true) {
        list_borrowed1 = REPY_IteratorHelper_BorrowCurrent(eval_list_iter1);
        REPY_FOREACH_BREAK(eval_list_iter1);
    }

    validate("REPY_FOREACH_BREAK releases borrowed handle", list_borrowed1 != REPY_NO_OBJECT && !REPY_IsValidHandle(list_borrowed1));

    REPY_FOREACH_FNAC(eval_list_iter2, list_handle1) {
        list_borrowed1 = REPY_IteratorHelper_BorrowCurrent(eval_list_iter2);
        break;
    }

    // Quick test of deferred reference release behavior.
    REPY_Handle deferred_cleanup_handle = REPY_FN_DEFER_RELEASE(REPY_CreateBool("True"));
    bool deferred_valid_before = REPY_IsValidHandle(deferred_cleanup_handle);
    REPY_FN_CLEANUP; 
    bool deferred_valid_after = REPY_IsValidHandle(deferred_cleanup_handle);

    validate("REPY_DeferredCleanupHandler released a deferred handle", deferred_valid_before && !deferred_valid_after);
    validate("REPY_FOREACH_FNAC cleaned up by REPY_DeferredCleanupHandler", list_borrowed1 != REPY_NO_OBJECT && !REPY_IsValidHandle(list_borrowed1));
}