#pragma once

extern "C" {
    #include "mod_recomp.h"
    #define RECOMP_API_VERSION 1
    #define TO_PTR(type, var) ((type*)(&rdram[(uint64_t)var - 0xFFFFFFFF80000000]))
    #define PTR(x) int32_t

    // Type Defs:
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;
    typedef void (*RecompExtlibFunction)(uint8_t* rdram, recomp_context* ctx);
}



#include <string>
#include <stdint.h>

#define RDRAM_TO_PTR(rdram, type, var) ((type*)(&rdram[(uint64_t)var - 0xFFFFFFFF80000000]))
#define RDRAM_FROM_PTR(rdram, ptr) (0x80000000 + ((int32_t)(((uint8_t*)ptr) - ((uint8_t*)rdram))))
#define TO_PTR(type, var) ((type*)(&rdram[(uint64_t)var - 0xFFFFFFFF80000000]))
#define FROM_PTR(ptr) (0x80000000 + ((int32_t)(((uint8_t*)ptr) - ((uint8_t*)rdram))))
#define PTR(x) int32_t

#if defined(_WIN32)
    #define DLLEXPORT __declspec(dllexport)
    #define DLLIMPORT __declspec(dllimport)
#else
    #define DLLEXPORT __attribute__((visibility("default")))
    #define DLLIMPORT
#endif

inline std::string ptr_to_string_n(uint8_t* rdram, size_t len, PTR(char) str) {
    std::string ret{};
    ret.reserve(len + 1);

    size_t total = 0;
    for (size_t i = 0; i < len; i++) {
        char next = (char)MEM_B(str, i);
        if (next == 0x00) {
            break;
        }
        total++;
        ret += next;
    }
    ret.resize(total);
    return ret;
}

inline int32_t string_to_ptr(uint8_t* rdram, std::string str, PTR(char) dest) {
    int32_t total = 0;
    for (size_t i = 0; i <= str.size(); i++) {
	if (i == str.size()) {
		// Last run of the loop.
		MEM_B(dest, i) = 0x00; 
	} else {
        	MEM_B(dest, i) = str.at(i);
	}
        total++;
    }
    return total;
}

inline std::u8string ptr_to_u8string_n(uint8_t* rdram, size_t len, PTR(char) str) {
    std::u8string ret{};
    ret.reserve(len + 1);

    size_t total = 0;
    for (size_t i = 0; i < len; i++) {
        char next = (char)MEM_B(str, i);
        if (next == 0x00) {
            break;
        }
        total++;
        ret += next;
    }
    ret.resize(total);

    return ret;
}

inline int32_t u8string_to_ptr(uint8_t* rdram, std::u8string str, PTR(char) dest) {
    int32_t total = 0;
    for (size_t i = 0; i <= str.size(); i++) {
	if (i == str.size()) {
		// Last run of the loop.
		MEM_B(dest, i) = 0x00; 
	} else {
        	MEM_B(dest, i) = str.at(i);
	}
        total++;
    }
    return total;
}


inline std::string ptr_to_string(uint8_t* rdram, PTR(char) str) {
    size_t len = 0;
    while (MEM_B(str, len) != 0x00) {
        len++;
    }
    return ptr_to_string_n(rdram, len, str);
}

inline std::u8string ptr_to_u8string(uint8_t* rdram, PTR(char) str) {
    size_t len = 0;
    while (MEM_B(str, len) != 0x00) {
        len++;
    }
    return ptr_to_u8string_n(rdram, len, str);
}

template<int index, typename T>
T _arg(uint8_t* rdram, recomp_context* ctx) {
    static_assert(index < 4, "Only args 0 through 3 supported");
    gpr raw_arg = (&ctx->r4)[index];
    if constexpr (std::is_same_v<T, float>) {
        if constexpr (index < 2) {
            static_assert(index != 1, "Floats in arg 1 not supported");
            return ctx->f12.fl;
        }
        else {
            // static_assert in else workaround
            [] <bool flag = false>() {
                static_assert(flag, "Floats in a2/a3 not supported");
            }();
        }
    }
    else if constexpr (std::is_pointer_v<T>) {
        static_assert (!std::is_pointer_v<std::remove_pointer_t<T>>, "Double pointers not supported");
        return TO_PTR(std::remove_pointer_t<T>, raw_arg);
    }
    else if constexpr (std::is_integral_v<T>) {
        static_assert(sizeof(T) <= 4, "64-bit args not supported");
        return static_cast<T>(raw_arg);
    }
    else {
        // static_assert in else workaround
        [] <bool flag = false>() {
            static_assert(flag, "Unsupported type");
        }();
    }
}

template<int index, typename T>
void _set_arg(uint8_t* rdram, recomp_context* ctx, T value) {
    static_assert(index < 4, "Only args 0 through 3 supported");
    // gpr raw_arg = (&ctx->r4)[index];
    if constexpr (std::is_same_v<T, float>) {
        if constexpr (index < 2) {
            static_assert(index != 1, "Floats in arg 1 not supported");
            ctx->f12.fl = value;
        }
        else {
            // static_assert in else workaround
            [] <bool flag = false>() {
                static_assert(flag, "Floats in a2/a3 not supported");
            }();
        }
    }
    else if constexpr (std::is_pointer_v<T>) {
        static_assert (!std::is_pointer_v<std::remove_pointer_t<T>>, "Double pointers not supported");
	(&ctx->r4)[index] = FROM_PTR(value);
    }
    else if constexpr (std::is_integral_v<T>) {
        static_assert(sizeof(T) <= 4, "64-bit args not supported");
        (&ctx->r4)[index] = static_cast<T>(value);
    }
    else {
        // static_assert in else workaround
        [] <bool flag = false>() {
            static_assert(flag, "Unsupported type");
        }();
    }
}

template <int arg_index>
std::string _arg_string(uint8_t* rdram, recomp_context* ctx) {
    PTR(char) str = _arg<arg_index, PTR(char)>(rdram, ctx);

    // Get the length of the byteswapped string.
    return ptr_to_string(rdram, str);
}

template <int arg_index>
int32_t _set_arg_string(uint8_t* rdram, recomp_context* ctx, PTR(char) ptr, std::string value) {
    int32_t retVal = string_to_ptr(rdram, value, ptr);
    _set_arg<arg_index, PTR(char)>(rdram, ctx, ptr);
    return retVal;
}


template <int arg_index>
std::u8string _arg_u8string(uint8_t* rdram, recomp_context* ctx) {
    PTR(char) str = _arg<arg_index, PTR(char)>(rdram, ctx);

    // Get the length of the byteswapped string.
    return ptr_to_u8string(rdram, str);
}

template <int arg_index>
int32_t _set_arg_u8string(uint8_t* rdram, recomp_context* ctx, PTR(char) ptr, std::u8string value) {
    int32_t retVal = u8string_to_ptr(rdram, value, ptr);
    _set_arg<arg_index, PTR(char)>(rdram, ctx, ptr);
    return retVal;
}

template <int arg_index>
std::string _arg_string_n(uint8_t* rdram, recomp_context* ctx, size_t len) {
    PTR(char) str = _arg<arg_index, PTR(char)>(rdram, ctx);

    // Get the length of the byteswapped string.
    return ptr_to_string_n(rdram, len, str);
}

template <int arg_index>
std::u8string _arg_u8string_n(uint8_t* rdram, recomp_context* ctx, size_t len) {
    PTR(char) str = _arg<arg_index, PTR(char)>(rdram, ctx);

    // Get the length of the byteswapped string.
    return ptr_to_u8string_n(rdram, len, str);
}

template <typename T>
void _return(recomp_context* ctx, T val) {
    static_assert(sizeof(T) <= 4 && "Only 32-bit value returns supported currently");
    if constexpr (std::is_same_v<T, float>) {
        ctx->f0.fl = val;
    }
    else if constexpr (std::is_integral_v<T> && sizeof(T) <= 4) {
        ctx->r2 = int32_t(val);
    }
    else {
        // static_assert in else workaround
        [] <bool flag = false>() {
            static_assert(flag, "Unsupported type");
        }();
    }
}

template <typename T>
T _get_return(uint8_t* rdram, recomp_context* ctx) {
    static_assert(sizeof(T) <= 4 && "Only 32-bit value returns supported currently");
    if constexpr (std::is_same_v<T, float>) {
        return ctx->f0.fl;
    }
    else if constexpr (std::is_integral_v<T> && sizeof(T) <= 4) {
        return (T) ctx->r2;
    } 
    else if constexpr (std::is_pointer_v<T>) {
        static_assert (!std::is_pointer_v<std::remove_pointer_t<T>>, "Double pointers not supported");
	return TO_PTR(std::remove_pointer_t<T>, ctx->r2);
    }
    else {
        // static_assert in else workaround
        [] <bool flag = false>() {
            static_assert(flag, "Unsupported type");
        }();
    }
}

inline void memcpy_from_recomp(uint8_t* rdram, void* dst, PTR(uint8_t) src, size_t size) {
    uint8_t* _dst = (uint8_t*)dst;
    for (size_t i = 0; i < size; i++) {
        _dst[i] = MEM_B(src, i);
    }
}

inline void memcpy_to_recomp(uint8_t* rdram, PTR(uint8_t) dst, void* src, size_t size) {
    uint8_t* _src = (uint8_t*)src;
    for (size_t i = 0; i < size; i++) {
        MEM_B(dst, i) = _src[i];
    }
}

inline void memcpy_rev_from_recomp(uint8_t* rdram, void* dst, PTR(uint8_t) src, size_t size) {
    uint8_t* _dst = (uint8_t*)dst;
    for (size_t i = 0; i < size; i++) {
        _dst[size - i - 1] = MEM_B(src, i);
    }
}

inline void memcpy_rev_to_recomp(uint8_t* rdram, PTR(uint8_t) dst, void* src, size_t size) {
    uint8_t* _src = (uint8_t*)src;
    for (size_t i = 0; i < size; i++) {
        MEM_B(dst, i) = _src[size - i - 1];
    }
}

#define NO_EXTERN_RECOMP_DLL_FUNC(_f_name) RECOMP_EXPORT void _f_name(uint8_t* rdram, recomp_context* ctx)
#define RECOMP_DLL_FUNC(_f_name) extern "C" NO_EXTERN_RECOMP_DLL_FUNC(_f_name)
#define RECOMP_ARG(_type, _pos) _arg<_pos, _type>(rdram, ctx)
#define RECOMP_ARG_STR(_pos) _arg_string<_pos>(rdram, ctx)
#define RECOMP_ARG_U8STR(_pos) _arg_u8string<_pos>(rdram, ctx)
#define RECOMP_ARG_STR_N(_pos, len) _arg_string_n<_pos>(rdram, ctx, len)
#define RECOMP_ARG_U8STR_N(_pos, len) _arg_u8string_n<_pos>(rdram, ctx, len)
#define RECOMP_RETURN(_type, _value) _return(ctx, (_type) _value); return
