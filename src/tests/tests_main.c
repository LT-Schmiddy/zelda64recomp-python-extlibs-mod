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
// Also checking if REPY_Len gives the same result as str[n]len during the non-SUH macros
#define CREATE_CAST_STR_TEST_BLOCK(c_type, py_type_create, py_type_cast, value_str, max_size) \
{ \
    char* value = value_str; \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value); \
    validate("Create" #py_type_create " - REPY_Len(test_handle) == strnlen(value)", REPY_Len(test_handle) == strnlen(value, max_size)); \
    char* test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Cast" # py_type_cast " - REPY_Len(test_handle) == strnlen(test_val)", REPY_Len(test_handle) == strnlen(test_val, max_size)); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    REPY_Release(test_handle); \
    recomp_free(test_val); \
} 

#define CREATE_CAST_STR_TEST_BLOCK_SUH(c_type, py_type_create, py_type_cast, value_str, max_size) \
{ \
    char* value = value_str; \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value); \
    char* test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    validate("Handle of Create" #py_type_create " is no longer valid", !REPY_IsValidHandle(test_handle)); \
    recomp_free(test_val); \
} 

#define CREATE_CAST_STRN_TEST_BLOCK(c_type, py_type_create, py_type_cast, value_str, max_size) \
{ \
    char* value = value_str; \
    u32 val_len = strnlen(value, max_size); \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value, val_len); \
    validate("Create" #py_type_create " - REPY_Len(test_handle) == strnlen(value)", REPY_Len(test_handle) == strnlen(value, max_size)); \
    char* test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Cast" # py_type_cast " - REPY_Len(test_handle) == strnlen(test_val)", REPY_Len(test_handle) == strnlen(test_val, max_size)); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    REPY_Release(test_handle); \
    recomp_free(test_val); \
} 

#define CREATE_CAST_STRN_TEST_BLOCK_SUH(c_type, py_type_create, py_type_cast, value_str, max_size) \
{ \
    char* value = value_str; \
    u32 val_len = strnlen(value, max_size); \
    REPY_Handle test_handle = REPY_Create ## py_type_create(value, val_len); \
    char* test_val = REPY_Cast ## py_type_cast(test_handle); \
    validate("Create" #py_type_create "/Cast" # py_type_cast " with " #c_type, strncmp(test_val, value, max_size) == 0); \
    validate("Handle of Create" #py_type_create " is no longer valid", !REPY_IsValidHandle(test_handle)); \
    recomp_free(test_val); \
} 

#define CREATE_CAST_STR_TEST(py_type, value, max_size) \
CREATE_CAST_STR_TEST_BLOCK(char*, py_type, py_type, value, max_size) \
CREATE_CAST_STRN_TEST_BLOCK(char*, py_type ## N, py_type, value, max_size) \
CREATE_CAST_STR_TEST_BLOCK_SUH(char*, py_type ## _SUH, py_type, value, max_size) \
CREATE_CAST_STRN_TEST_BLOCK_SUH(char*, py_type ## N_SUH, py_type, value, max_size) 


REPY_ON_INIT void REPY_API_Tests() {
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
    // From here, we assume that handle operations work.

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
    
    // Also Tests that REPY_Len works with strings.
    CREATE_CAST_STR_TEST(Str, "HELLO WORLD!", 20);
    CREATE_CAST_STR_TEST(ByteStr, "HELLO WORLD!", 20);
    // From here, we assume that Create/Cast operations work.

    // Dicts are needed for execution contexts. Test those next.
    REPY_Handle test_dict = REPY_CreateEmptyDict_SUH();
    validate("REPY_CreateEmptyDict_SUH - test_dict created and is empty", REPY_Len(test_dict) == 0);
    validate("REPY_CreateEmptyDict_SUH - test_dict is just invalidated", REPY_IsValidHandle(test_dict) == false);

    // Testing Dict Get/Set/Has/Del
    test_dict = REPY_CreateEmptyDict();
    s32 test_value_1_val = 999;
    REPY_Handle test_key_1 = REPY_CreateStr("test_key_1");
    validate("REPY_CreateEmptyDict: test_dict re-created and is empty", REPY_Len(test_dict) == 0);
    REPY_DictSet(test_dict, test_key_1, REPY_CreateS32_SUH(test_value_1_val));
    validate("REPY_DictSet - REPY_Len(test_dict) == 1", REPY_Len(test_dict) == 1);
    validate("REPY_DictHas(test_key_1) == true", REPY_DictHas(test_dict, test_key_1) == true);
    REPY_Handle test_value_1 = REPY_DictGet(test_dict, test_key_1);
    validate("REPY_DictGet - test_value_1_val == REPY_CastS32(test_value_1)", test_value_1_val == REPY_CastS32(test_value_1));
    REPY_DictDel(test_dict, test_key_1);
    validate("REPY_DictDel - REPY_DictHas(test_key_1) == false", REPY_DictHas(test_dict, test_key_1) == false);
    validate("REPY_DictDel - test_dict is empty again", REPY_Len(test_dict) == 0);

    s32 test_value_2_val = 888;
    char* test_key_2 = "test_key_2";
    REPY_DictSet_CStr(test_dict, test_key_2, REPY_CreateS32_SUH(test_value_2_val));
    validate("REPY_DictSet_CStr - REPY_Len(test_dict) == 1", REPY_Len(test_dict) == 1);
    validate("REPY_DictHas_CStr(test_key_2) == true", REPY_DictHas_CStr(test_dict, test_key_2) == true);
    REPY_Handle test_value_2 = REPY_DictGet_CStr(test_dict, test_key_2);
    validate("REPY_DictGet_CStr - test_value_2_val == REPY_CastS32(test_value_2)", test_value_2_val == REPY_CastS32(test_value_2));
    REPY_DictDel_CStr(test_dict, test_key_2);
    validate("REPY_DictDel_CStr - REPY_DictHas_CStr(test_key_2) == false", REPY_DictHas_CStr(test_dict, test_key_2) == false);
    validate("REPY_DictDel_CStr - test_dict is empty again", REPY_Len(test_dict) == 0);
    // Admittedly, we only tested a dict with a single entry. Might need to expand that later.
    REPY_Release(test_dict);
    REPY_Release(test_key_1);
    REPY_Release(test_value_1);
    REPY_Release(test_value_2);
    // From here on, we'll assume that these dict operations are working as expected.
    // The REPY_CreateDict variadic function isn't needed for Python code execution, and depends on Tuple construction. So we'll test that later.

    // The following tests of executing Python code rely on the assumption that Python itself is working correctly.
    // Testing code execution via CStrings. 
    REPY_Handle py_globals = REPY_CreateEmptyDict();
    REPY_Handle py_locals = REPY_CreateEmptyDict();
    REPY_Handle test_var_x_name = REPY_CreateStr("x");
    validate("REPY_ExecCStr - 'x = 5' ran without error", REPY_ExecCStr("x = 5", py_globals, py_locals));
    validate("REPY_ExecCStr - 'x' is in local scope", REPY_DictHas(py_locals, test_var_x_name));
    validate("REPY_ExecCStr - 'x' is NOT in global scope", !REPY_DictHas(py_globals, test_var_x_name));
    validate("REPY_ExecCStr - 'x' assigned to 5", 5 == REPY_CastS32(REPY_MakeSUH(REPY_DictGet(py_locals, test_var_x_name))));
    validate("REPY_EvalCStr - 'x == 5' is true", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("x == 5", py_globals, py_locals))));
    validate("REPY_EvalCStr - 'x == 6' is false", !REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("x == 6", py_globals, py_locals))));

    // Repeat these with N functions and different vars:
    REPY_Handle test_var_y_name = REPY_CreateStr("y");
    // The purpose of these functions is to not need null-terminated code strings. So we'll set the length args to exclude that.
    validate("REPY_ExecCStr - 'y = 8' ran without error", REPY_ExecCStrN("y = 8", 5, py_globals, py_locals));
    validate("REPY_ExecCStr - 'y' is in local scope", REPY_DictHas(py_locals, test_var_y_name));
    validate("REPY_ExecCStr - 'y' is NOT in global scope", !REPY_DictHas(py_globals, test_var_y_name));
    validate("REPY_ExecCStr - 'y' assigned to 8", 8 == REPY_CastS32(REPY_MakeSUH(REPY_DictGet(py_locals, test_var_y_name))));
    validate("REPY_EvalCStr - 'y == 8' is true", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStrN("y == 8", 6, py_globals, py_locals))));
    validate("REPY_EvalCStr - 'y == 9' is false", !REPY_CastBool(REPY_MakeSUH(REPY_EvalCStrN("y == 9", 6, py_globals, py_locals))));
    // We'll need more of the API validated before we can test error handling. That will come after all other execution stuff is tested.
    // From here on, we will assume that running Python code directly from strings works as expected, provided the Python code is correct.
    REPY_Release(test_var_x_name);
    REPY_Release(test_var_y_name);
    // Now to test pre-compilation and caching of Python code.
    REPY_Handle py_exec_string =  REPY_CreateStr("exec");
    REPY_Handle a_assign_bytecode = REPY_Compile(REPY_CreateStr_SUH("a = 20"), REPY_CreateStr_SUH("REPY_Compile exec test: a = 20"), py_exec_string);
    validate("REPY_Compile - exec 'a = 20' compiled successfully", a_assign_bytecode != REPY_NO_OBJECT);
    REPY_Handle b_assign_bytecode = REPY_CompileCStr("b = 30", "REPY_Compile exec test: b = 30", REPY_CODE_EXEC);
    validate("REPY_CompileCStr - exec 'b = 30' compiled successfully", b_assign_bytecode != REPY_NO_OBJECT);
    REPY_Handle c_assign_bytecode = REPY_CompileCStrN("c = 40", 6, "REPY_Compile exec test: c = 40", REPY_CODE_EXEC);
    validate("REPY_CompileCStrN - exec 'c = 40' compiled successfully", c_assign_bytecode != REPY_NO_OBJECT);
    
    REPY_Handle py_eval_string =  REPY_CreateStr("eval");
    REPY_Handle a_eval_bytecode = REPY_Compile(REPY_CreateStr_SUH("a == 20"), REPY_CreateStr_SUH("REPY_Compile eval test: a == 20"), py_eval_string);
    validate("REPY_Compile - eval 'a == 20' compiled successfully", a_assign_bytecode != REPY_NO_OBJECT);
    REPY_Handle b_eval_bytecode = REPY_CompileCStr("b == 30", "REPY_Compile eval test: b == 30", REPY_CODE_EVAL);
    validate("REPY_CompileCStr - eval 'b == 30' compiled successfully", b_assign_bytecode != REPY_NO_OBJECT);
    REPY_Handle c_eval_bytecode = REPY_CompileCStrN("c == 40", 7, "REPY_Compile eval test: c == 40", REPY_CODE_EVAL);
    validate("REPY_CompileCStrN - eval 'c == 40' compiled successfully", c_assign_bytecode != REPY_NO_OBJECT);
    
    // Testing execution of the code:
    validate("REPY_Exec - exec 'a = 20' executed successfully", REPY_Exec(a_assign_bytecode, py_globals, py_locals));
    validate("REPY_Exec - exec 'b = 30' executed successfully", REPY_Exec(b_assign_bytecode, py_globals, py_locals));
    validate("REPY_Exec - exec 'c = 40' executed successfully", REPY_Exec(c_assign_bytecode, py_globals, py_locals));
    validate("REPY_Exec - eval 'a == 20' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(a_eval_bytecode, py_globals, py_locals))));
    validate("REPY_Exec - eval 'b == 30' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(b_eval_bytecode, py_globals, py_locals))));
    validate("REPY_Exec - eval 'c == 40' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(c_eval_bytecode, py_globals, py_locals))));

    REPY_Release(py_exec_string);
    REPY_Release(a_assign_bytecode);
    REPY_Release(b_assign_bytecode);
    REPY_Release(c_assign_bytecode);
    REPY_Release(py_eval_string);
    REPY_Release(a_eval_bytecode);
    REPY_Release(b_eval_bytecode);
    REPY_Release(c_eval_bytecode);

    // Lets check that REPY_Exec and REPY_Eval can use Python strings as well:
    validate("REPY_Exec - exec str 'd = 100' executed successfully", REPY_Exec(REPY_CreateStr_SUH("d = 100"), py_globals, py_locals));
    validate("REPY_Exec - eval str 'd == 100' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("d == 100"), py_globals, py_locals))));
    validate("REPY_Exec - exec str 'e = 1000' executed successfully", REPY_Exec(REPY_CreateStr_SUH("e = 1000"), py_globals, py_locals));
    validate("REPY_Exec - eval str 'e == 1000' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("e == 1000"), py_globals, py_locals))));

    // From here on, we'll assume that compiling and executing bytecode, as well as executing python strings works correctly, so long as the Python code is correct.
    // Testing the Memcpy functions:
    char memcpy_test_cstr[13] = "Hello World!";
    REPY_Handle test_bstr = REPY_CreateByteStr(memcpy_test_cstr);
    REPY_Handle memcpy_test_byte_str = REPY_MemcpyToByteStr(memcpy_test_cstr, 12, false); // Don't include the null-terminator for this
    REPY_Handle memcpy_test_byte_array = REPY_MemcpyToByteArray(memcpy_test_cstr, 12, false); // Don't include the null-terminator for this
    REPY_DictSet_CStr(py_locals, "memcpy_test_byte_str", memcpy_test_byte_str);
    REPY_DictSet_CStr(py_locals, "test_bstr", test_bstr);
    REPY_DictSet_CStr(py_locals, "memcpy_test_byte_array", memcpy_test_byte_array);
    validate("REPY_MemcpyToByteStr - eval 'memcpy_test_byte_str == test_bstr' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("memcpy_test_byte_str == test_bstr"), py_globals, py_locals))));
    REPY_ExecCStr("memcpy_test_byte_array2 = bytearray(memcpy_test_byte_str)", py_globals, py_locals);
    validate("REPY_MemcpyToByteArray - eval 'memcpy_test_byte_array == memcpy_test_byte_array2' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("memcpy_test_byte_array == memcpy_test_byte_array"), py_globals, py_locals))));

    // Testing writing back into recomp memory:
    char memcpy_from_test [13];
    memcpy_from_test [12] = 0; 
    REPY_MemcpyFromByteStr(memcpy_from_test, 12, false, memcpy_test_byte_str);
    validate("REPY_MemcpyFromByteStr - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(memcpy_from_test, memcpy_test_cstr, 12) == 0);
    REPY_MemcpyFromByteArray(memcpy_from_test, 12, false, memcpy_test_byte_array);
    validate("REPY_MemcpyFromByteArray - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(memcpy_from_test, memcpy_test_cstr, 12) == 0);
    u32 alloc_write_size = 0;
    char* alloc_byte_str = REPY_AllocAndCopyByteStr(false, memcpy_test_byte_str, &alloc_write_size);
    validate("REPY_AllocAndCopyByteStr - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(alloc_byte_str, memcpy_test_cstr, 12) == 0);
    validate("REPY_AllocAndCopyByteStr - alloc_write_size == 12", alloc_write_size == 12);
    char* alloc_byte_array = REPY_AllocAndCopyByteArray(false, memcpy_test_byte_array, &alloc_write_size);
    validate("REPY_AllocAndCopyByteArray - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(alloc_byte_array, memcpy_test_cstr, 12) == 0);
    validate("REPY_AllocAndCopyByteArray - alloc_write_size == 12", alloc_write_size == 12);
    recomp_free(alloc_byte_str);
    recomp_free(alloc_byte_array);
    REPY_Release(test_bstr);
    REPY_Release(memcpy_test_byte_str);
    REPY_Release(memcpy_test_byte_array);

    // From here on, we'll assume the memcpy functions are working correctly.
    // Testing index lookup:
    REPY_Handle py_tuple1 = REPY_EvalCStr("(0, 1, 2, 3, 4, 5)", 0, 0);
    bool py_list_match = true;
    for (int i = 0; i < 6; i++) {
        py_list_match = py_list_match && (i == REPY_CastS32(REPY_MakeSUH(REPY_TupleGetIndexS32(py_tuple1, i))));
    }
    validate("REPY_TupleGetIndexS32 returned correct values for (0, 1, 2, 3, 4, 5)", py_list_match);
    REPY_Release(py_tuple1);
    // From here on, we'll assume that REPY_TupleGetIndexS32 is working.
    // Testing Tuple construction.
    py_list_match = true;
    REPY_Handle py_tuple2 = REPY_CreateTuple(4, REPY_CreateS32_SUH(3), REPY_CreateS32_SUH(2), REPY_CreateS32_SUH(1), REPY_CreateS32_SUH(0));

    for (int i = 3; i >= 0; i--) {
        py_list_match = py_list_match && (i == REPY_CastS32(REPY_MakeSUH(REPY_TupleGetIndexS32(py_tuple2, 3 - i))));
    }
    validate("REPY_TupleGetIndexS32 returned correct values for (3, 2, 1, 0) created with REPY_CreateTuple", py_list_match);
    REPY_Release(py_tuple2);

    REPY_Release(py_globals);
    REPY_Release(py_locals);
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);

}

