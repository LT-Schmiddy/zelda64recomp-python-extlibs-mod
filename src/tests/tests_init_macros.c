#include "tests_init_macros.h"
#include "tests_main.h"
/*
REPY_GLOBAL_COMPILE_CACHE(REPY_MAIN_INTERPRETER, global_code_cache, REPY_CODE_EXEC, "x = 10");
REPY_STATIC_COMPILE_CACHE(REPY_MAIN_INTERPRETER, static_code_cache, REPY_CODE_EXEC, "x = 20");
REPY_GLOBAL_COMPILE_INCBIN_CACHE(REPY_MAIN_INTERPRETER, global_incbin_cache, "global_incbin_test.py");
REPY_STATIC_COMPILE_INCBIN_CACHE(REPY_MAIN_INTERPRETER, static_incbin_cache, "static_incbin_test.py");

#define COMPILE_CACHE_CHECK(identifier_name, eval_statement) \
bool identifier_name ## _is_valid = REPY_IsValidHandle(identifier_name); \
validate("" #identifier_name " is a valid handle.", identifier_name ## _is_valid); \
bool identifier_name ## _executed = false; \
if (identifier_name ## _is_valid) { \
    identifier_name ## _executed = REPY_FN_EXEC(identifier_name); \
} \
validate("" #identifier_name " executed properly.", identifier_name ## _executed); \
validate("" #identifier_name " manipulated the scope as expected.", identifier_name ## _executed && REPY_FN_EVAL_CSTR_BOOL(eval_statement)); \
*/
void run_main_init_macro_tests() {
    REPY_FN_SETUP;

    // COMPILE_CACHE_CHECK(global_code_cache, "x == 10");
    // COMPILE_CACHE_CHECK(static_code_cache, "x == 20");
    // COMPILE_CACHE_CHECK(global_incbin_cache, "x == 5");
    // COMPILE_CACHE_CHECK(static_incbin_cache, "x == 15");

    REPY_FN_CLEANUP;
} 