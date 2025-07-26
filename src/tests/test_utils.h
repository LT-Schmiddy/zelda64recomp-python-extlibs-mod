#ifndef __TEST_UTILS__
#define __TEST_UTILS__
#include "global.h"

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char* str);
size_t strnlen(const char* str, size_t maxlen);

#endif