#include "tests_fn_macros.h"
#include "tests_main.h"

#include "PR/os_thread.h"
#include "stack.h"

// Convienience Macros:
#define THREAD_DATA(identifier, size, id_num) \
STACK(stack_ ## identifier, size); \
OSThread identifier; \
OSId id_ ## identifier = id_num; 

#define THREAD_FUNC(identifier) \
void entry_ ## identifier (void* args)

#define THREAD_CREATE(identifier, args_ptr, priority) \
osCreateThread(&identifier, id_ ## identifier, entry_ ## identifier, args_ptr, STACK_TOP(stack_ ## identifier), priority)

//
THREAD_DATA(testThread1, 0x500, 120);
THREAD_FUNC(testThread1) {
    for (int i = 0; i < 100; i++) {
        recomp_printf("Running test thread 1 - %i\n", i);
    }
}

THREAD_DATA(testThread2, 0x500, 121);
THREAD_FUNC(testThread2) {
    for (int i = 0; i < 100; i++) {
        recomp_printf("Running test thread 2 - %i\n", i);
    }
}

void run_concurrency_tests() {
    THREAD_CREATE(testThread1, NULL, 21);
    THREAD_CREATE(testThread2, NULL, 20);
    osStartThread(&testThread1);
    osStartThread(&testThread2);


}