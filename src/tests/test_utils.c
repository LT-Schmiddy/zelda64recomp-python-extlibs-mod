#include "test_utils.h"
#include "modding.h"
#include "recomputils.h"

#include "repy_api.h" 
 

// Implementation borrowed from https://github.com/embeddedartistry/libc
int strcmp(const char* s1, const char* s2)
{
	int r = -1;

	if(s1 == s2)
	{
		// short circuit - same string
		return 0;
	}

	// I don't want to panic with a NULL ptr - we'll fall through and fail w/ -1
	if(s1 != NULL && s2 != NULL)
	{
		// iterate through strings until they don't match or s1 ends (null-term)
		for(; *s1 == *s2; ++s1, ++s2)
		{
			if(*s1 == 0)
			{
				r = 0;
				break;
			}
		}

		// handle case where we didn't break early - set return code.
		if(r != 0)
		{
			r = *(const char*)s1 - *(const char*)s2;
		}
	}
	return r;
}

// Implementation borrowed from https://github.com/embeddedartistry/libc
int strncmp(const char* s1, const char* s2, size_t n)
{
	int r = -1;
	if(s1 == s2)
	{
		// short circuit - same string
		return 0;
	}
	// I don't want to panic with a NULL ptr - we'll fall through and fail
	if(s1 != NULL && s2 != NULL)
	{
		// iterate through strings until they don't match, s1 ends, or n == 0
		for(; n && *s1 == *s2; ++s1, ++s2, n--)
		{
			if(*s1 == 0)
			{
				r = 0;
				break;
			}
		}
		// handle case where we didn't break early - set return code.
		if(n == 0)
		{
			r = 0;
		}
		else if(r != 0)
		{
			r = *s1 - *s2;
		}
	}
	return r;
}

// #if LONG_BIT == 32
static const unsigned long mask01 = 0x01010101;
static const unsigned long mask80 = 0x80808080;
// #elif LONG_BIT == 64
// static const unsigned long mask01 = 0x0101010101010101;
// static const unsigned long mask80 = 0x8080808080808080;
// #else
// #error Unsupported word size
// #endif

#define LONGPTR_MASK (sizeof(long) - 1)

/*
 * Helper macro to return string length if we caught the zero
 * byte.
 */
#define testbyte(x)                                     \
	do                                                  \
	{                                                   \
		if(p[x] == '\0')                                \
			return ((uintptr_t)p - (uintptr_t)str + x); \
	} while(0)

size_t strlen(const char* str)
{
	const char* p;
	const unsigned long* lp;

	/* Skip the first few bytes until we have an aligned p */
	for(p = str; (uintptr_t)p & LONGPTR_MASK; p++)
	{
		if(*p == '\0')
		{
			return ((uintptr_t)p - (uintptr_t)str);
		}
	}

	/* Scan the rest of the string using word sized operation */
	// Cast to void to prevent alignment warning
	for(lp = (const unsigned long*)(const void*)p;; lp++)
	{
		if((*lp - mask01) & mask80)
		{
			p = (const char*)(lp);
			testbyte(0);
			testbyte(1);
			testbyte(2);
			testbyte(3);
#if(LONG_BIT >= 64)
			testbyte(4);
			testbyte(5);
			testbyte(6);
			testbyte(7);
#endif
		}
	}

	/* NOTREACHED */
	// return (0);
}

size_t strnlen(const char* str, size_t maxlen)
{
	const char* cp;

	for(cp = str; maxlen != 0 && *cp != '\0'; cp++, maxlen--)
	{
		;
	}

	return (size_t)(cp - str);
}