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

// Patches a function in the base game that's used to check if the player should quickspin.
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

    REPY_FN_EXEC_BLOCK(
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
