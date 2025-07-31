#ifndef __MOD_LOGGING__
#define __MOD_LOGGING__
#include "modding.h"
#include "global.h"
#include "extlib_functions.h"
// Config Area:
#define RECOMP_CONFIG_LOG_LEVEL recomp_get_config_u32("log_level")

// End of config:
#ifndef __FILE_NAME__
#define __FILE_NAME__ "Unknown Source File"
#endif

typedef enum {
    RECOMP_LOG_NONE = 0,
    RECOMP_LOG_FATAL= 1,
    RECOMP_LOG_ERROR = 2,
    RECOMP_LOG_WARNING = 3,
    RECOMP_LOG_INFO= 4,
    RECOMP_LOG_DEBUG = 5,
    RECOMP_LOG_VERBOSE = 6
} RecompLogLevel;

#define IS_LOG_LEVEL(log_level) log_level <= RECOMP_CONFIG_LOG_LEVEL
#define IF_LOG_LEVEL(log_level) if (IS_LOG_LEVEL(log_level))

#define IS_LOG_FATAL IS_LOG_LEVEL(RECOMP_LOG_FATAL)
#define IS_LOG_ERROR IS_LOG_LEVEL(RECOMP_LOG_ERROR)
#define IS_LOG_WARNING IS_LOG_LEVEL(RECOMP_LOG_WARNING)
#define IS_LOG_INFO IS_LOG_LEVEL(RECOMP_LOG_INFO)
#define IS_LOG_DEBUG IS_LOG_LEVEL(RECOMP_LOG_DEBUG)
#define IS_LOG_VERBOSE IS_LOG_LEVEL(RECOMP_LOG_VERBOSE)

#define IF_LOG_FATAL IF_LOG_LEVEL(RECOMP_LOG_FATAL)
#define IF_LOG_ERROR IF_LOG_LEVEL(RECOMP_LOG_ERROR)
#define IF_LOG_WARNING IF_LOG_LEVEL(RECOMP_LOG_WARNING)
#define IF_LOG_INFO IF_LOG_LEVEL(RECOMP_LOG_INFO)
#define IF_LOG_DEBUG IF_LOG_LEVEL(RECOMP_LOG_DEBUG)
#define IF_LOG_VERBOSE IF_LOG_LEVEL(RECOMP_LOG_VERBOSE)

#define _LOG_PRINTF_CALL(...) RECOMP_LOG_CONFIG_PRINTF_FUNC(__VA_ARGS__)

#define LOG_FORMAT(log_level, ...) \
mod_handle_log(log_level, __func__, __LINE__, __FILE_NAME__, __VA_ARGS__)

// Logging Macros:
#define LOG_FATAL(...) LOG_FORMAT(RECOMP_LOG_FATAL, __VA_ARGS__)
#define LOG_ERROR(...) LOG_FORMAT(RECOMP_LOG_ERROR, __VA_ARGS__)
#define LOG_WARNING(...) LOG_FORMAT(RECOMP_LOG_WARNING, __VA_ARGS__)
#define LOG_INFO(...) LOG_FORMAT(RECOMP_LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) LOG_FORMAT(RECOMP_LOG_DEBUG, __VA_ARGS__)
#define LOG_VERBOSE(...) LOG_FORMAT(RECOMP_LOG_VERBOSE, __VA_ARGS__)

// Short-hand Logging Macros:
#define LOGF(...) LOG_FATAL(__VA_ARGS__)
#define LOGE(...) LOG_ERROR(__VA_ARGS__)
#define LOGW(...) LOG_WARNING(__VA_ARGS__)
#define LOGI(...) LOG_INFO(__VA_ARGS__)
#define LOGD(...) LOG_DEBUG(__VA_ARGS__)
#define LOGV(...) LOG_VERBOSE(__VA_ARGS__)

void mod_handle_log(u32 severity, const char* func_name, u32 line_number, const char* file_name, const char* format, ...);

#endif