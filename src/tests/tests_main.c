#include "modding.h"
#include "global.h"
#include "recomputils.h"

#include "repy_api.h"
#include "test_utils.h"

REPY_INCBIN_MODULE(test_module, "test_module.py");


int _test_cases = 0;
int _test_cases_passed = 0; 

void validate(char* case_name, bool case_stmt) {
    _test_cases++;

    if (case_stmt) {
        _test_cases_passed++;
    }
    
    recomp_printf("Case %s %s\n", case_stmt ? "Passed:" : "Failed!", case_name);
}

// These will streamline the process of testing the primatives create/cast.
#define CREATE_CAST_TEST_BLOCK(c_type, py_type, value) \
{ \
    REPY_Handle test_handle = REPY_Create ## py_type(value); \
    c_type test_val = REPY_Cast ## py_type(test_handle); \
    validate("Create" #py_type "/Cast" # py_type " with " #c_type, test_val == value); \
    REPY_Release(test_handle); \
} 

#define CREATE_CAST_TEST_BLOCK_SUH(c_type, py_type_create, py_type_cast, value) \
{ \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value); \
    c_type test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, test_val == value); \
    validate("Handle of Create" #py_type_create " is no longer valid", !REPY_IsValidHandle(test_handle)); \
} 

#define CREATE_CAST_TEST(c_type, py_type, value) \
CREATE_CAST_TEST_BLOCK(c_type, py_type, value) \
CREATE_CAST_TEST_BLOCK_SUH(c_type, py_type ## _SUH, py_type, value) 

// String Versions:
#define CREATE_CAST_STR_TEST_BLOCK(c_type, py_type, value, max_size) \
{ \
    REPY_Handle test_handle = REPY_Create ## py_type(value); \
    char* test_val = REPY_Cast ## py_type(test_handle); \
    validate("Create" #py_type "/Cast" # py_type " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    REPY_Release(test_handle); \
    /* recomp_printf("test_val: '%s'", test_val); \
    recomp_printf("value: '%s'", value); */\
    recomp_free(test_val); \
} 

#define CREATE_CAST_STR_TEST_BLOCK_SUH(c_type, py_type_create, py_type_cast, value, max_size) \
{ \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value); \
    char* test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    validate("Handle of Create" #py_type_create " is no longer valid", !REPY_IsValidHandle(test_handle)); \
    /* recomp_printf("test_val: '%s'\n", test_val); \
    recomp_printf("value: '%s'\n", value); */ \
    recomp_free(test_val); \
} 

#define CREATE_CAST_STR_TEST(py_type, value, max_size) \
CREATE_CAST_STR_TEST_BLOCK(char*, py_type, value, max_size) \
CREATE_CAST_STR_TEST_BLOCK_SUH(char*, py_type ## _SUH, py_type, value, max_size) 


REPY_ON_INIT void REPY_Tests() {
    // Testing Handle Operations:
    REPY_Handle testbool = REPY_CreateBool(true);
    validate("First assigned handle (testbool) == 1", testbool == 1);
    validate("testbool is valid (REPY_IsValidHandle)", REPY_IsValidHandle(testbool));
    validate("testbool is not SUH (REPY_GetSUH)", REPY_GetSUH(testbool) == 0);
    REPY_MakeSUH(testbool);
    validate("testbool is made SUH (REPY_MakeSUH)", REPY_GetSUH(testbool) == 1);
    REPY_SetSUH(testbool, false);
    validate("testbool SUH disabled again (REPY_MakeSUH)", REPY_GetSUH(testbool) == 0);
    REPY_Handle testbool2 = REPY_CopyHandle(testbool);
    validate("Copied handle (testbool2) == 2", testbool2 == 2);
    REPY_MakeSUH(testbool2);
    REPY_Handle testbool3 = REPY_CopyHandle(testbool2);
    validate("testbool2 is not valid after SUH access", !REPY_IsValidHandle(testbool2));
    REPY_Release(testbool);
    validate("testbool is not valid after release (REPY_Release)", !REPY_IsValidHandle(testbool));
    REPY_Release(testbool3);

    // Testing Create/Casting of primatives:
    CREATE_CAST_TEST(u8, U8, 66);
    CREATE_CAST_TEST(u16, U16, 700);
    CREATE_CAST_TEST(u32, U32, 80000);
    CREATE_CAST_TEST(u64, U64, 9000000000);
    
    CREATE_CAST_TEST(s8, S8, -66);
    CREATE_CAST_TEST(s16, S16, -700);
    CREATE_CAST_TEST(s32, S32, -80000);
    CREATE_CAST_TEST(s64, S64, -9000000000);

    CREATE_CAST_TEST(f32, F32, 80000.5f);
    CREATE_CAST_TEST(f64, F64, 9000000000);

    CREATE_CAST_STR_TEST(Str, "HELLO WORLD!", 20);
    CREATE_CAST_STR_TEST(ByteStr, "HELLO WORLD!", 20);

    // Evaluation Tests. Doing these now since we'll need them later:
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
}