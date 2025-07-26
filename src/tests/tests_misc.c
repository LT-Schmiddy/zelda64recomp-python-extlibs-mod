#include "modding.h"
#include "global.h"
#include "recomputils.h"

#include "repy_api.h"

/*
void inline_test() {
    REPY_FN_SETUP;
    REPY_FN_IMPORT("os");

    REPY_FN_SET_S32("count", 55);

    REPY_Handle new_dict = REPY_CreateEmptyDict();
    REPY_DictSet(new_dict, REPY_MakeSUH(REPY_CreateStr("key_string")), REPY_MakeSUH(REPY_CreateStr("value_string")));
    REPY_FN_SET("new_dict", new_dict);
    REPY_Release(new_dict);

    // REPY_FN_EXEC_CACHE(
    //     inline_test_exec1, 
    //     "print(f'Hello Mr. {count=}')\n"
    //     "print(f'Hello Mr. {new_dict=}')\n"
        
    //     "for i in os.listdir():\n"
    //     "    print('*', i)\n"
    // );

    REPY_Handle os_handle = REPY_FN_GET("os");
    const char* os_name = REPY_CastStr(REPY_MakeSUH(REPY_CallAttrReturn(os_handle, "getcwd", 0, 0)));
    recomp_printf("Python says the CWD is '%s'\n", os_name);

    REPY_Release(os_handle);
    recomp_free((void*)os_name);

    REPY_FN_RETURN;
}

void file_access_test() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(
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


    REPY_FN_EVAL_CACHE(tuple_test_get_print, "print", print_fn);
    REPY_Handle args = REPY_CreateTuple(3, REPY_MakeSUH(REPY_CreateS32(0)), REPY_MakeSUH(REPY_CreateS32(1)), REPY_MakeSUH(REPY_CreateS32(2)));
    REPY_FN_SET("index_2", REPY_MakeSUH(REPY_TupleGetMember(args, 2)));

    REPY_Call(print_fn, args, 0);
    REPY_Release(args);
    REPY_Release(print_fn);
    REPY_FN_RETURN;
}

void mem_test() {
    REPY_FN_SETUP;
    const char* addr_test = "Hello Alex";
    REPY_FN_SET_S32("addr_test", (s32)addr_test);
    REPY_FN_EXEC_CACHE(
        mem_test_exec1, 
        "import recomp_mem\n"
        "print(type(addr_test))\n"
        "byte_info = recomp_mem.read_bytes_n(addr_test, 11)\n"
        // "print(byte_info)\n"
        "recomp_mem.write_bytes(addr_test, b'Perfect\\x00')\n"
    );

    recomp_printf("addr_test = %s\n", addr_test);
    REPY_FN_RETURN;
}

void time_test() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(
        time_start1,
        "import time\n"
        "start_time = time.time()\n"
    );

    for (int i = 0; i < 100; i++) {
        inline_test();
        file_access_test();
        tuple_test();
        mem_test();
    }


    REPY_FN_EXEC_CACHE(
        time_end1,
        "end_time = time.time() - start_time\n"
        "print(f'{end_time=}')\n"
    );
    REPY_FN_RETURN;
}

void print_dict_test_no_code() {
    REPY_Handle builtins = REPY_ImportModule("builtins");

    REPY_Handle my_dict = REPY_CreateDict(4,
        REPY_CreatePair_SUH(REPY_CreateStr_SUH("Key1"), REPY_CreateStr_SUH("Value1")),
        REPY_CreatePair_SUH(REPY_CreateStr_SUH("Key2"), REPY_CreateStr_SUH("Value2")),
        REPY_CreatePair_SUH(REPY_CreateStr_SUH("Key3"), REPY_CreateStr_SUH("Value3")),
        REPY_CreatePair_SUH(REPY_CreateStr_SUH("Key4"), REPY_CreateStr_SUH("Value4"))
    );

    REPY_CallAttr(builtins, "print", REPY_CreateTuple_SUH(1, my_dict), 0);
    REPY_Release(builtins);
    REPY_Release(my_dict);
}

void print_dict_test_code() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(
        code_test1, 
        "import builtins\n"
        "my_dict = {\n"
        "   'Key1': 'Value1',\n"
        "   'Key2': 'Value2',\n"
        "   'Key3': 'Value3',\n"
        "   'Key4': 'Value4'\n"
        "}\n"
        "builtins.print(my_dict)\n"
    );
    REPY_FN_RETURN;
}

void no_code_block_test() {
    REPY_FN_SETUP;
    REPY_FN_EXEC_CACHE(
        time_start1,
        "import time\n"
        "nc_start_time = time.time()\n"
    );

    for (int i = 0; i < 1000; i++) {
        print_dict_test_no_code();
    }

    REPY_FN_EXEC_CACHE(
        time_end1,
        "nc_run_time = time.time() - nc_start_time\n"
        "c_start_time = time.time()\n"
    );

    for (int i = 0; i < 1000; i++) {
        print_dict_test_code();
    }

    REPY_FN_EXEC_CACHE(
        time_end2,
        "c_run_time = time.time() - c_start_time\n"
        "print(f'{nc_run_time=}, {c_run_time=}')\n"
    );
    REPY_FN_RETURN;
}

// Patches a function in the base game that's used to check if the player should quickspin.
REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");

    recomp_printf("bool size %u\n", sizeof(bool));

    REPY_FN_SET_U64("u64val1", 99);
    REPY_FN_SET_S64("s64val1", -99);
    REPY_FN_SET_F64("f64val1", 99.5);
    REPY_FN_EXEC_CACHE(
        create_int1,
        "print(f'{u64val1=}')\n"
        "print(f'{s64val1=}')\n"
        "print(f'{f64val1=}')\n"
        "u64val2 = 33\n"
        "s64val2 = -33\n"
        "f64val2 = 33.0\n"
    );
    
    u64 u64val2 = REPY_FN_GET_U64("u64val2");
    s64 s64val2 = REPY_FN_GET_S64("s64val2");
    f64 f64val2 = REPY_FN_GET_F64("f64val2");
    
    recomp_printf("llu1 %llu (size %u)\n", u64val2, sizeof(u64val2));
    recomp_printf("llu2 %lli (size %u)\n", s64val2, sizeof(s64val2));
    recomp_printf("llu2 %llf (size %u)\n", f64val2, sizeof(f64val2));

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}

REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");
    // REPY_FN_IMPORT("test_module");
    u8 u8_test = 99;
    s8 s8_test = -99;
    u16 u16_test = 99;
    s16 s16_test = -99;
    u32 u32_test = 99;
    s32 s32_test = -99;
    u64 u64_test = 99;
    s64 s64_test = -99;
    f32 f32_test = 99.5f;
    f64 f64_test = -99.5;

    REPY_FN_SET_S32("u8_test_ptr", (s32)&u8_test);
    REPY_FN_SET_S32("s8_test_ptr", (s32)&s8_test);
    REPY_FN_SET_S32("u16_test_ptr", (s32)&u16_test);
    REPY_FN_SET_S32("s16_test_ptr", (s32)&s16_test);
    REPY_FN_SET_S32("u32_test_ptr", (s32)&u32_test);
    REPY_FN_SET_S32("s32_test_ptr", (s32)&s32_test);
    REPY_FN_SET_S32("u64_test_ptr", (s32)&u64_test);
    REPY_FN_SET_S32("s64_test_ptr", (s32)&s64_test);
    REPY_FN_SET_S32("f32_test_ptr", (s32)&f32_test);
    REPY_FN_SET_S32("f64_test_ptr", (s32)&f64_test);

    REPY_FN_EXEC_CACHE(
        recomp_mem_test1,
        "import recomp_mem\n"
        "print(f'{recomp_mem.read_u8(u8_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_s8(s8_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_u16(u16_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_s16(s16_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_u32(u32_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_s32(s32_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_u64(u64_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_s64(s64_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_f32(f32_test_ptr)=}')\n"
        "print(f'{recomp_mem.read_f64(f64_test_ptr)=}')\n"
        "recomp_mem.write_u8(u8_test_ptr, 33)\n"
        "recomp_mem.write_s8(s8_test_ptr, -33)\n"
        "recomp_mem.write_u16(u16_test_ptr, 33)\n"
        "recomp_mem.write_s16(s16_test_ptr, -33)\n"
        "recomp_mem.write_u32(u32_test_ptr, 33)\n"
        "recomp_mem.write_s32(s32_test_ptr, -33)\n"
        "recomp_mem.write_u64(u64_test_ptr, 33)\n"
        "recomp_mem.write_s64(s64_test_ptr, -33)\n"
        "recomp_mem.write_f32(f32_test_ptr, 33.5)\n"
        "recomp_mem.write_f64(f64_test_ptr, -33.5)\n"
        "\n"
    );

    recomp_printf("u8_test = %u\n", u8_test);
    recomp_printf("s8_test = %i\n", s8_test);
    recomp_printf("u16_test = %u\n", u16_test);
    recomp_printf("s16_test = %i\n", s16_test);
    recomp_printf("u32_test = %u\n", u32_test);
    recomp_printf("s32_test = %i\n", s32_test);
    recomp_printf("u64_test = %llu\n", u64_test);
    recomp_printf("s64_test = %lli\n", s64_test);
    recomp_printf("f32_test = %f\n", f32_test);
    recomp_printf("f64_test = %lf\n", f64_test);

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}

REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");

    u8 bytes_array[10] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
    REPY_Handle bytes_test = REPY_MemcpyToByteArray(bytes_array, 10, false);
    REPY_FN_SET("bytes_test", bytes_test);

    REPY_FN_EXEC_CACHE(
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


REPY_ON_INIT void REPY_Tests() {
    REPY_FN_SETUP;
    recomp_printf("REPY Tests Loaded\n");
        REPY_FN_EXEC_CACHE(
        time_start1,
        "import time\n"
        "nc_start_time = time.time()\n"
    );

    REPY_FN_EXEC_CACHE(
        bad_fib1,
        "count_holder = 0\n"
        "def count_step():\n"
        "    global count_holder\n"
        "    count_holder += 1\n"
        "\n"
    );
    
    u64* fib_table = recomp_alloc(sizeof(u64)* FIB_TABLE_SIZE);
    REPY_Handle fib_fn = REPY_FN_GET("count_step");

    for (int i = 0; i < FIB_TABLE_SIZE; i++) {
        REPY_Call(fib_fn, 0, 0);
        fib_table[i] = REPY_FN_GET_U64("count_holder");
        if (i % (FIB_TABLE_SIZE / 10) == 0) {
            recomp_printf("=");
        }
    }
    
    REPY_FN_EXEC_CACHE(
        time_end1,
        "fib_run_time = time.time() - nc_start_time\n"
    );
    recomp_printf("\n... ");
    for (int i = 0; i < 5; i++) {
        recomp_printf("%llu, ", fib_table[FIB_TABLE_SIZE - 6 + i]);
    }

    REPY_FN_EXEC_CACHE(
        time_print1,
        "print(f'{fib_run_time=}')\n"
    );
    recomp_free(fib_table);

    // no_code_block_test();
    recomp_printf("REPY: Passed %i out of %i cases.\n", _test_cases_passed, _test_cases);
    REPY_FN_CLEANUP;
}



REPY_ON_INIT void run_misc_tests() {
    REPY_FN_SETUP;
    REPY_FN_SET_S32("x", 32);

    REPY_FN_IF(if_test1, 1, "x == 0") {
        recomp_printf("bytecode_index -> 0\n");
    } REPY_FN_ELIF(if_test1, "x == 32") {
        recomp_printf("bytecode_index -> 1\n");
    } else {
        recomp_printf("bytecode ran else statement\n");
    }
    
    REPY_FN_FOREACH(foreach_test1, "i", "[1, 2, 3, 4, 5]") {
        REPY_FN_EXEC_CACHE(
            foreach_print_test,
            "print(i)\n"
        );
    }

    REPY_FN_FOR(for_test1, "i = 0", "i < 10", "i += 1") {
        REPY_FN_EXEC_CACHE(
            for_print_test,
            "print(i)\n"
        );
    }

    REPY_FN_CLEANUP;
}
*/