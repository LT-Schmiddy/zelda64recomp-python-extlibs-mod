#include "modding.h"
#include "global.h"
#include "recomputils.h"

#include "repy_api.h"
#include "test_utils.h"

// REPY_INCBIN_MODULE(test_module, "test_module.py");
PRE_INIT_ADD_NRM_TO_MODULE_PATH;

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

#define REPY_MEM_TEST(c_type, py_create, value) \
{ \
    c_type value_var = value; \
    REPY_DictSetCStr(py_locals, "test_ptr", REPY_CreatePtr_SUH(&value_var)); \
    REPY_DictSetCStr(py_locals, "test_value", REPY_MakeSUH(REPY_Create ## py_create (value))); \
    validate("repy_api.mem.read_" #c_type " works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_" #c_type "(test_ptr) == test_value", py_globals, py_locals)))); \
    REPY_ExecCStr("mem.write_" #c_type "(test_ptr, test_value + 1)", py_globals, py_locals); \
    validate("repy_api.mem.write_" #c_type " works", value_var == value + 1); \
} \

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

    CREATE_CAST_TEST(void*, Ptr, NULL);
    
    CREATE_CAST_TEST(f32, F32, 80000.5f);
    CREATE_CAST_TEST(f64, F64, 9000000000);
    
    // Also Tests that REPY_Len works with strings.
    CREATE_CAST_STR_TEST(Str, "HELLO WORLD!", 20);
    CREATE_CAST_STR_TEST(ByteStr, "HELLO WORLD!", 20);
    // From here, we assume that Create/Cast operations work.

    // Dicts are needed for execution contexts. Test those next.
    REPY_Handle test_dict = REPY_CreateDict_SUH(0);
    validate("REPY_CreateEmptyDict_SUH -> test_dict created and is empty", REPY_Len(test_dict) == 0);
    validate("REPY_CreateEmptyDict_SUH -> test_dict is just invalidated", REPY_IsValidHandle(test_dict) == false);

    // Testing Dict Get/Set/Has/Del
    // test_dict = REPY_CreateEmptyDict();
    test_dict = REPY_CreateDict(0);
    s32 test_value_1_val = 999;
    REPY_Handle test_key_1 = REPY_CreateStr("test_key_1");
    validate("REPY_CreateEmptyDict: test_dict re-created and is empty", REPY_Len(test_dict) == 0);
    REPY_DictSet(test_dict, test_key_1, REPY_CreateS32_SUH(test_value_1_val));
    validate("REPY_DictSet -> REPY_Len(test_dict) == 1", REPY_Len(test_dict) == 1);
    validate("REPY_DictHas(test_key_1) == true", REPY_DictHas(test_dict, test_key_1) == true);
    REPY_Handle test_value_1 = REPY_DictGet(test_dict, test_key_1);
    validate("REPY_DictGet -> test_value_1_val == REPY_CastS32(test_value_1)", test_value_1_val == REPY_CastS32(test_value_1));
    REPY_DictDel(test_dict, test_key_1);
    validate("REPY_DictDel -> REPY_DictHas(test_key_1) == false", REPY_DictHas(test_dict, test_key_1) == false);
    validate("REPY_DictDel -> test_dict is empty again", REPY_Len(test_dict) == 0);

    s32 test_value_2_val = 888;
    char* test_key_2 = "test_key_2";
    REPY_DictSetCStr(test_dict, test_key_2, REPY_CreateS32_SUH(test_value_2_val));
    validate("REPY_DictSetCStr - REPY_Len(test_dict) == 1", REPY_Len(test_dict) == 1);
    validate("REPY_DictHasCStr(test_key_2) == true", REPY_DictHasCStr(test_dict, test_key_2) == true);
    REPY_Handle test_value_2 = REPY_DictGetCStr(test_dict, test_key_2);
    validate("REPY_DictGetCStr - test_value_2_val == REPY_CastS32(test_value_2)", test_value_2_val == REPY_CastS32(test_value_2));
    REPY_DictDelCStr(test_dict, test_key_2);
    validate("REPY_DictDelCStr - REPY_DictHasCStr(test_key_2) == false", REPY_DictHasCStr(test_dict, test_key_2) == false);
    validate("REPY_DictDelCStr - test_dict is empty again", REPY_Len(test_dict) == 0);
    // Admittedly, we only tested a dict with a single entry. Might need to expand that later.
    REPY_Release(test_dict);
    REPY_Release(test_key_1);
    REPY_Release(test_value_1);
    REPY_Release(test_value_2);
    // From here on, we'll assume that these dict operations are working as expected.
    // The REPY_CreateDict variadic function isn't needed for Python code execution, and depends on Tuple construction. So we'll test that later.

    // The following tests of executing Python code rely on the assumption that Python itself is working correctly.
    // Testing code execution via CStrings. 
    REPY_Handle py_globals = REPY_CreateDict(0);
    REPY_Handle py_locals = REPY_CreateDict(0);
    REPY_Handle test_var_x_name = REPY_CreateStr("x");
    validate("REPY_ExecCStr -> 'x = 5' ran without error", REPY_ExecCStr("x = 5", py_globals, py_locals));
    validate("REPY_ExecCStr -> 'x' is in local scope", REPY_DictHas(py_locals, test_var_x_name));
    validate("REPY_ExecCStr -> 'x' is NOT in global scope", !REPY_DictHas(py_globals, test_var_x_name));
    validate("REPY_ExecCStr -> 'x' assigned to 5", 5 == REPY_CastS32(REPY_MakeSUH(REPY_DictGet(py_locals, test_var_x_name))));
    validate("REPY_EvalCStr -> 'x == 5' is true", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("x == 5", py_globals, py_locals))));
    validate("REPY_EvalCStr -> 'x == 6' is false", !REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("x == 6", py_globals, py_locals))));

    // Repeat these with N functions and different vars:
    REPY_Handle test_var_y_name = REPY_CreateStr("y");
    // The purpose of these functions is to not need null-terminated code strings. So we'll set the length args to exclude that.
    validate("REPY_ExecCStr -> 'y = 8' ran without error", REPY_ExecCStrN("y = 8", 5, py_globals, py_locals));
    validate("REPY_ExecCStr -> 'y' is in local scope", REPY_DictHas(py_locals, test_var_y_name));
    validate("REPY_ExecCStr -> 'y' is NOT in global scope", !REPY_DictHas(py_globals, test_var_y_name));
    validate("REPY_ExecCStr -> 'y' assigned to 8", 8 == REPY_CastS32(REPY_MakeSUH(REPY_DictGet(py_locals, test_var_y_name))));
    validate("REPY_EvalCStr -> 'y == 8' is true", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStrN("y == 8", 6, py_globals, py_locals))));
    validate("REPY_EvalCStr -> 'y == 9' is false", !REPY_CastBool(REPY_MakeSUH(REPY_EvalCStrN("y == 9", 6, py_globals, py_locals))));
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
    REPY_Handle memcpy_test_byte_str = REPY_MemcpyToBytes(memcpy_test_cstr, 12, false); // Don't include the null-terminator for this
    REPY_Handle memcpy_test_byte_array = REPY_MemcpyToByteArray(memcpy_test_cstr, 12, false); // Don't include the null-terminator for this
    REPY_DictSetCStr(py_locals, "memcpy_test_byte_str", memcpy_test_byte_str);
    REPY_DictSetCStr(py_locals, "test_bstr", test_bstr);
    REPY_DictSetCStr(py_locals, "memcpy_test_byte_array", memcpy_test_byte_array);
    validate("REPY_MemcpyToBytes - eval 'memcpy_test_byte_str == test_bstr' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("memcpy_test_byte_str == test_bstr"), py_globals, py_locals))));
    REPY_ExecCStr("memcpy_test_byte_array2 = bytearray(memcpy_test_byte_str)", py_globals, py_locals);
    validate("REPY_MemcpyToByteArray - eval 'memcpy_test_byte_array == memcpy_test_byte_array2' evaluated true", REPY_CastBool(REPY_MakeSUH(REPY_Eval(REPY_CreateStr_SUH("memcpy_test_byte_array == memcpy_test_byte_array"), py_globals, py_locals))));

    // Testing writing back into recomp memory:
    char memcpy_from_test [13];
    memcpy_from_test [12] = 0; 
    REPY_MemcpyFromBytes(memcpy_from_test, 12, false, memcpy_test_byte_str);
    validate("REPY_MemcpyFromBytes - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(memcpy_from_test, memcpy_test_cstr, 12) == 0);
    REPY_MemcpyFromByteArray(memcpy_from_test, 12, false, memcpy_test_byte_array);
    validate("REPY_MemcpyFromByteArray - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(memcpy_from_test, memcpy_test_cstr, 12) == 0);
    u32 alloc_write_size = 0;
    char* alloc_byte_str = REPY_AllocAndCopyBytes(false, memcpy_test_byte_str, &alloc_write_size);
    validate("REPY_AllocAndCopyBytes - strncmp(memcpy_from_test, memcpy_test_cstr) is true", strncmp(alloc_byte_str, memcpy_test_cstr, 12) == 0);
    validate("REPY_AllocAndCopyBytes - alloc_write_size == 12", alloc_write_size == 12);
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

    // validate("REPY_TupleGetIndexS32 returned correct values for (0, 1, 2, 3, 4, 5)", py_list_match);
    // for (int i = -1; i > -7; i--) {
    //     py_list_match = py_list_match && ((i + 6) == REPY_CastS32(REPY_MakeSUH(REPY_TupleGetIndexS32(py_tuple1, i))));
    // }
    // validate("REPY_TupleGetIndexS32 returned correct values for (0, 1, 2, 3, 4, 5) using negatives", py_list_match);

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

    // Testing function invokations. We'll use the built-in `int` function for that.
    REPY_Handle py_int_fn = REPY_EvalCStr("int", 0, 0);
    validate("REPY_Call ran without error", REPY_Call(py_int_fn, REPY_MakeSUH(REPY_CreateTuple(1, REPY_CreateByteStr_SUH("33"))), 0));
    REPY_Handle py_int_return1 = REPY_CallReturn(py_int_fn, REPY_MakeSUH(REPY_CreateTuple(1, REPY_CreateByteStr_SUH("33"))), 0);
    validate("REPY_CallReturn ran without error", py_int_return1);
    validate("py_int_return1 == 33", py_int_return1 && REPY_CastS32(py_int_return1) == 33);
    REPY_Release(py_int_fn);
    REPY_Release(py_int_return1);
    // For calling attributes, we'll the attributes of an int object:
    REPY_Handle py_test_int = REPY_CreateU32(99);
    validate("REPY_CallAttr ran without error", REPY_CallAttr(py_test_int, REPY_CreateStr("bit_length"), 0, 0));
    validate("REPY_CallAttrCStr ran without error", REPY_CallAttrCStr(py_test_int,"bit_length", 0, 0));

    REPY_Handle py_int_return2 = REPY_CallAttrReturn(py_test_int, REPY_CreateStr("bit_length"), 0, 0);
    validate("REPY_CallAttr_Return ran without error", py_int_return2);
    validate("py_int_return2 == 7", py_int_return2 && REPY_CastS32(py_int_return2) == 7);
    REPY_Release(py_int_return2);

    REPY_Handle py_int_return3 = REPY_CallAttrCStrReturn(py_test_int,"bit_length", 0, 0);
    validate("REPY_CallAttrCStrReturn ran without error", py_int_return3);
    validate("py_int_return3 == 7", py_int_return3 && REPY_CastS32(py_int_return3) == 7);
    REPY_Release(py_int_return3);
    // From here on, we'll assume that function calls work correctly.
    REPY_Release(py_test_int);
    
    // Testing module loading and attribute manipulation:
    REPY_Handle py_os_module = REPY_ImportModule("os");
    validate("Python module 'os' imported without error", py_os_module);

    REPY_Handle py_cwd = REPY_CallAttrCStrReturn(py_os_module, "getcwd", 0, 0);
    validate("os.getcwd() returns without error", py_cwd);


    // Testing object attribute manipulation using using the OS module as the object.
    s32 obj_test_value_1_val = 999;
    REPY_Handle obj_test_key_1 = REPY_CreateStr("obj_test_key_1");
    REPY_SetAttr(py_os_module, obj_test_key_1, REPY_CreateS32_SUH(obj_test_value_1_val));
    validate("REPY_HasAttr(test_key_1) == true", REPY_HasAttr(py_os_module, obj_test_key_1) == true);
    REPY_Handle obj_test_value_1 = REPY_GetAttr(py_os_module, obj_test_key_1, 0);
    validate("REPY_GetAttr -> test_value_1_val == REPY_CastS32(test_value_1)", obj_test_value_1_val == REPY_CastS32(obj_test_value_1));
    REPY_DelAttr(py_os_module, obj_test_key_1);
    validate("REPY_DelAttr -> REPY_HasAttr(test_key_1) == false", REPY_HasAttr(py_os_module, obj_test_key_1) == false);

    // Testing CStr versions:
    s32 obj_test_value_2_val = 999;
    char* obj_test_key_2_cstr = "obj_test_key_1";
    REPY_SetAttrCStr(py_os_module, obj_test_key_2_cstr, REPY_CreateS32_SUH(obj_test_value_2_val));
    validate("REPY_SetAttrCStr-> REPY_HasAttrCStr(test_key_1) == true", REPY_HasAttrCStr(py_os_module, obj_test_key_2_cstr) == true);
    REPY_Handle obj_test_value_2 = REPY_GetAttrCStr(py_os_module, obj_test_key_2_cstr, 0);
    validate("REPY_GetAttrCStr -> test_value_1_val == REPY_CastS32(test_value_1)", obj_test_value_2_val == REPY_CastS32(obj_test_value_2));
    REPY_DelAttrCStr(py_os_module, obj_test_key_2_cstr);
    validate("REPY_DelAttrCStr -> REPY_HasAttrCStr(test_key_1) == false", REPY_HasAttrCStr(py_os_module, obj_test_key_2_cstr) == false);
    // We'll assume that object attribute manipulation works from here on.
    REPY_Release(py_os_module);
    REPY_Release(py_cwd);
    REPY_Release(obj_test_key_1);
    REPY_Release(obj_test_value_1);
    REPY_Release(obj_test_value_2);

    // I guess we can consider the stdlib modules to be working. Let's try with NRM modules.
    REPY_Handle py_repy_api = REPY_ImportModule("repy_api");
    validate("Python module from NRM 'repy_api' imported without error", py_repy_api);
    validate("repy_api has member 'version_str'", REPY_HasAttrCStr(py_repy_api, "version_str"));

    // What about incbinned modules:
    // REPY_Handle py_test_module = REPY_ImportModule("test_module");
    // validate("INCBIN Python module 'test_module' imported without error", py_test_module);
    // validate("repy_api has member 'test_string'", REPY_HasAttrCStr(py_test_module, "test_string"));
    // From here on, we'll assume that all module functionality works.

    REPY_Release(py_repy_api);
    // REPY_Release(py_test_module);

    REPY_Handle iter_test_tuple = REPY_EvalCStr("(0, 1, 2, 3, 4, 5, 6)", 0, 0);
    REPY_Handle iter_test_dict = REPY_CreateDict(0);

    u32 iter_array[7] = {0, 1, 2, 3, 4, 5, 6};
    bool iter_index_works = true;
    bool iter_curr_works = true;
    bool iter_scope_works = true;
    char* var_name = "val";
    REPY_Handle iter_handles[10];

    for (REPY_IteratorHelper* iter = REPY_IteratorHelper_Create(iter_test_tuple, iter_test_dict, var_name); REPY_IteratorHelper_Update(iter, true);) {
        iter_index_works = iter_index_works && (iter_array[iter->index] == iter->index);
        iter_curr_works = iter_curr_works && (iter_array[iter->index] == REPY_CastU32(iter->curr));
        iter_scope_works = iter_scope_works && (iter_array[iter->index] == REPY_CastU32(REPY_MakeSUH(REPY_DictGetCStr(iter_test_dict, var_name))));

        iter_handles[iter->index] = iter->curr;
        iter_handles[7] = iter->iter;
        iter_handles[8] = iter->var_name;
        iter_handles[9] = iter->py_scope;
    }

    validate("REPY_IteratorHelper -> index updated properly", iter_index_works);
    validate("REPY_IteratorHelper -> curr updated properly", iter_curr_works);
    validate("REPY_IteratorHelper -> scope updated properly", iter_scope_works);
    bool iter_released_all = true;
    for (int i = 0; i < 10; i++) {
        iter_released_all = iter_released_all && !REPY_IsValidHandle(iter_handles[i]);
    }
    validate("REPY_IteratorHelper -> all handles released automatically", iter_released_all);
    // Checking what happens if we kill the handler early.

    REPY_Handle check_iter;
    REPY_Handle check_curr;
    REPY_Handle check_py_scope;
    REPY_Handle check_var_name;
    bool iter_break_works = true;
    for (REPY_IteratorHelper* iter = REPY_IteratorHelper_Create(iter_test_tuple, iter_test_dict, var_name); REPY_IteratorHelper_Update(iter, true);) {
        check_iter = iter->iter;
        check_curr = iter->curr;
        check_py_scope = iter->var_name;
        check_var_name = iter->py_scope;

        REPY_IteratorHelper_Destroy(iter);
        break;
    }

    iter_break_works = iter_released_all && !REPY_IsValidHandle(check_iter);
    iter_break_works = iter_released_all && !REPY_IsValidHandle(check_curr);
    iter_break_works = iter_released_all && !REPY_IsValidHandle(check_py_scope);
    iter_break_works = iter_released_all && !REPY_IsValidHandle(check_var_name);

    validate("REPY_IteratorHelper -> early cleanup works", iter_break_works);
    // From here on, we assume the iterator helper works.

    // Creating a value to check:
    REPY_DictSetCStr(py_locals, "if_check", REPY_CreateS32_SUH(2));

    // Testing the IfStmtHelper:
    static REPY_IfStmtChain* if_helper_chain_root = NULL;
    REPY_IfStmtHelper if_helper1;
    REPY_IfStmtHelper_InitInPlace(&if_helper1, &if_helper_chain_root);

    int step_result1 = -1;

    if (REPY_IfStmtHelper_Step(&if_helper1, py_globals, py_locals, "if_check == 0", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper1")) {
        step_result1 = 0;
    } else if (REPY_IfStmtHelper_Step(&if_helper1, py_globals, py_locals, "if_check == 1", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper1")) {
        step_result1 = 1;
    } else if (REPY_IfStmtHelper_Step(&if_helper1, py_globals, py_locals, "if_check == 2", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper1")) {
        step_result1 = 2;
    } else if (REPY_IfStmtHelper_Step(&if_helper1, py_globals, py_locals, "if_check == 3", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper1")) {
        step_result1 = 3;
    }

    validate("REPY_IfStmtHelper -> if_helper1 steps correctly", step_result1 == 2);
    validate("REPY_IfStmtHelper -> if_helper1 initialized the chain", if_helper_chain_root != NULL);

    // Let's inspect the if_helper_chain_root. 3 links should have been created.
    int if_helper_chain_depth = 0;
    bool no_invalid_bytcode_handles = true;
    REPY_IfStmtChain* if_helper_chain_current_link = if_helper_chain_root;
    while (if_helper_chain_current_link != NULL) {
        if_helper_chain_depth++;
        no_invalid_bytcode_handles = no_invalid_bytcode_handles && REPY_IsValidHandle(if_helper_chain_current_link->eval_expression_bytecode);
        if_helper_chain_current_link = if_helper_chain_current_link->next;
    }
    validate("REPY_IfStmtHelper -> if_helper_chain_root has 3 links", if_helper_chain_depth == 3);
    validate("REPY_IfStmtHelper -> all eval expressions are valid", no_invalid_bytcode_handles);

    // Checking that the chain isn't being reconstructed on repeated use:
    // Start by finding all the pointers and handles for each link:
    REPY_IfStmtChain** if_helper_chain_link_array = recomp_alloc(sizeof(REPY_IfStmtChain*) * if_helper_chain_depth);
    REPY_Handle* if_helper_chain_handle_array = recomp_alloc(sizeof(REPY_Handle) * if_helper_chain_depth);
    if_helper_chain_current_link = if_helper_chain_root; // we can reuse this variable.
    for (int i = 0; i < if_helper_chain_depth; i++) {
        if_helper_chain_link_array[i] = if_helper_chain_current_link;
        if_helper_chain_handle_array[i] = if_helper_chain_current_link->eval_expression_bytecode;
        if_helper_chain_current_link = if_helper_chain_current_link->next;
    }

    REPY_IfStmtHelper if_helper2;
    REPY_IfStmtHelper_InitInPlace(&if_helper2, &if_helper_chain_root);

    int step_result2 = -1;
    if (REPY_IfStmtHelper_Step(&if_helper2, py_globals, py_locals, "if_check == 0", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper2")) {
        step_result2 = 0;
    } else if (REPY_IfStmtHelper_Step(&if_helper2, py_globals, py_locals, "if_check == 1", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper2")) {
        step_result2 = 1;
    } else if (REPY_IfStmtHelper_Step(&if_helper2, py_globals, py_locals, "if_check == 2", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper2")) {
        step_result2 = 2;
    } else if (REPY_IfStmtHelper_Step(&if_helper2, py_globals, py_locals, "if_check == 3", __FILE_NAME__, (char*)__func__, __LINE__, "if_helper2")) {
        step_result2 = 3;
    }

    validate("REPY_IfStmtHelper -> if_helper1 steps correctly", step_result2 == 2);
    bool if_helper_same_chain = true;
    if_helper_chain_current_link = if_helper_chain_root; // we can reuse this variable.
    for (int i = 0; i < if_helper_chain_depth; i++) {
        if_helper_same_chain = 
            if_helper_same_chain 
            && (if_helper_chain_current_link == if_helper_chain_link_array[i]) 
            && (if_helper_chain_current_link->eval_expression_bytecode == if_helper_chain_handle_array[i])
        ;
        if_helper_chain_current_link = if_helper_chain_current_link->next;
    }
    validate("REPY_IfStmtHelper -> the if chain was not re-initialized after first use.", if_helper_same_chain);
    // From here on, we can assume the REPY_IfStmtHelper works.
    // Checking error handling:

    // Execute Something that throws an error:
    validate("Error Handling -> Starting with no Python error raised", !REPY_IsErrorSet());
    REPY_ExecCStr("pront('hello world')", py_globals, py_locals);
    validate("Error Handling -> Python error has been captured", REPY_IsErrorSet());

    REPY_DictSetCStr(py_locals, "error_trace1", REPY_MakeSUH(REPY_GetErrorTrace()));
    REPY_DictSetCStr(py_locals, "error_type1", REPY_MakeSUH(REPY_GetErrorType()));
    REPY_DictSetCStr(py_locals, "error_value1", REPY_MakeSUH(REPY_GetErrorValue()));

    validate("Error Handling -> error_trace1 is not None", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_trace1 is not None", py_globals, py_locals))));
    validate("Error Handling -> error_type1 is not None",  REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_type1 is not None", py_globals, py_locals))));
    validate("Error Handling -> error_value1 is not None",  REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_value1 is not None", py_globals, py_locals))));
    validate("Error Handling -> isinstance(error_value1, error_type1)",  REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("isinstance(error_value1, error_type1)", py_globals, py_locals))));

    REPY_ClearError();
    validate("Error Handling -> Python error has been released", !REPY_IsErrorSet());
    REPY_DictSetCStr(py_locals, "error_trace1", REPY_MakeSUH(REPY_GetErrorTrace()));
    REPY_DictSetCStr(py_locals, "error_type1", REPY_MakeSUH(REPY_GetErrorType()));
    REPY_DictSetCStr(py_locals, "error_value1", REPY_MakeSUH(REPY_GetErrorValue()));
    validate("Error Handling -> error_trace1 is None", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_trace1 is None", py_globals, py_locals))));
    validate("Error Handling -> error_type1 is None",  REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_type1 is None", py_globals, py_locals))));
    validate("Error Handling -> error_value1 is None",  REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("error_value1 is None", py_globals, py_locals))));
    // Handling thrown errors works. We'll test that every potential error thrower works correctly another time.


    // Let's test the repy_api.mem functions.
    REPY_ExecCStr("from repy_api import mem", py_globals, py_locals);


    REPY_MEM_TEST(u8, U8, 66);
    REPY_MEM_TEST(u16, U16, 700);
    REPY_MEM_TEST(u32, U32, 80000);
    REPY_MEM_TEST(u64, U64, 9000000000);
    
    REPY_MEM_TEST(s8, S8, -66);
    REPY_MEM_TEST(s16, S16, -700);
    REPY_MEM_TEST(s32, S32, -80000);
    REPY_MEM_TEST(s64, S64, -9000000000);
    
    REPY_MEM_TEST(f32, F32, 80000.5f);
    REPY_MEM_TEST(f64, F64, 9000000000);
    // Testing char
    char string_char[2] = "c";
    REPY_DictSetCStr(py_locals, "test_ptr", REPY_CreatePtr_SUH((void*)&string_char));
    REPY_DictSetCStr(py_locals, "test_value", REPY_MakeSUH(REPY_CreateStr((const char*)&string_char)));
    validate("repy_api.mem.read_char works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_char(test_ptr) == test_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_char(test_ptr, 'd')", py_globals, py_locals);
    validate("repy_api.mem.write_char works", string_char[0] == 'd');

    // Testing byte char
    char string_byte_char[2] = "c";
    REPY_DictSetCStr(py_locals, "test_ptr", REPY_CreatePtr_SUH((void*)&string_byte_char));
    REPY_DictSetCStr(py_locals, "test_value", REPY_MakeSUH(REPY_CreateByteStr((const char*)&string_byte_char)));
    validate("repy_api.mem.read_byte_char works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_byte_char(test_ptr) == test_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_byte_char(test_ptr, b'd')", py_globals, py_locals);
    validate("repy_api.mem.write_byte_char works", string_byte_char[0] == 'd');

    // testing str
    char string_str[50] = "hello world";
    REPY_DictSetCStr(py_locals, "test_ptr", REPY_CreatePtr_SUH((void*)&string_str));
    REPY_DictSetCStr(py_locals, "test_value", REPY_MakeSUH(REPY_CreateStr((const char*)&string_str)));
    validate("repy_api.mem.read_str works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_str(test_ptr) == test_value", py_globals, py_locals))));
    validate("repy_api.mem.read_str_n works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_str_n(test_ptr, 50) == test_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_str_n(test_ptr, 'hello recomp', 50)", py_globals, py_locals);
    validate("repy_api.mem.write_str_n writes the correct length", strnlen((const char*)string_str, 50) == 12);
    validate("repy_api.mem.write_str_n matches target", strncmp((const char*)string_str, "hello recomp", 50) == 0);

    // testing bytes str
    char string_byte_str[50] = "hello world";
    REPY_DictSetCStr(py_locals, "test_ptr", REPY_CreatePtr_SUH((void*)&string_byte_str));
    REPY_DictSetCStr(py_locals, "test_value", REPY_MakeSUH(REPY_CreateByteStr((const char*)&string_byte_str)));
    validate("repy_api.mem.read_byte_str works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_byte_str(test_ptr) == test_value", py_globals, py_locals))));
    validate("repy_api.mem.read_byte_str_n works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_byte_str_n(test_ptr, 50) == test_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_byte_str_n(test_ptr, b'hello recomp', 50)", py_globals, py_locals);
    validate("repy_api.mem.write_byte_str_n writes the correct length", strnlen((const char*)string_byte_str, 50) == 12);
    validate("repy_api.mem.write_byte_str_n matches target", strncmp((const char*)string_byte_str, "hello recomp", 50) == 0);

    // testing memcpy stuff.
    char mem_bytes[20] = "hello world";
    REPY_DictSetCStr(py_locals, "mem_bytes_ptr", REPY_CreatePtr_SUH((void*)&mem_bytes));
    REPY_DictSetCStr(py_locals, "mem_bytes_value", REPY_MakeSUH(REPY_MemcpyToBytes(mem_bytes, 20, false)));
    validate("repy_api.mem.read_bytes_n works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_bytes_n(mem_bytes_ptr, 20) == mem_bytes_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_bytes_n(mem_bytes_ptr, b'hello recomp', 20)", py_globals, py_locals);
    validate("repy_api.mem.write_bytes_n matches target", strncmp((const char*)mem_bytes, "hello recomp", 20) == 0);
    

    REPY_DictSetCStr(py_locals, "mem_bytearray_value", REPY_MakeSUH(REPY_MemcpyToByteArray(mem_bytes, 20, false)));
    validate("repy_api.mem.read_bytearray_n works", REPY_CastBool(REPY_MakeSUH(REPY_EvalCStr("mem.read_bytearray_n(mem_bytes_ptr, 20) == mem_bytearray_value", py_globals, py_locals))));
    REPY_ExecCStr("mem.write_bytearray_n(mem_bytes_ptr, bytearray(b'hello recomp'), 20)", py_globals, py_locals);
    validate("repy_api.mem.write_bytearray_n matches target", strncmp((const char*)string_byte_str, "hello recomp", 20) == 0);

    REPY_Release(py_globals);
    REPY_Release(py_locals);
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);

    
    {
        REPY_FN_SETUP;
        REPY_FN_SET_S32("test_cases", _test_cases);
        REPY_FN_SET_S32("test_cases_passed", _test_cases_passed);
        REPY_FN_EXEC_CSTR(
            "from pathlib import Path\n"
            "Path('./test_results.txt').write_text(f'Passed {test_cases_passed} of {test_cases} test cases.')\n"
        );

        REPY_FN_CLEANUP;
    }
    

    // recomp_printf("Starting Interavtive Shell. Call `exit()` to continue to game...\n");
    // REPY_Handle code_module = REPY_ImportModule("code");
    // REPY_CallAttrCStr(code_module, "interact", 0, 0);
    // REPY_ClearError();
    // REPY_Release(code_module);
}

