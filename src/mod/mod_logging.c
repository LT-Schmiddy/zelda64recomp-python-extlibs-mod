#include "mod_logging.h"
#include "printf.h"

void mod_handle_log(u32 severity, const char* func_name, u32 line_number, const char* file_name, const char* format, ...) {
    PythonNative_SetLogMetaData(severity, func_name, line_number, file_name);
    va_list len_va;
    va_start(len_va, format);
    u32 len = vlenprintf_(format, len_va);

    char* buf = recomp_alloc(len + 1);
    va_list str_va;
    va_start(str_va, format);
    vsnprintf_(buf, len + 1, format, str_va);
    PythonNative_CommitLogMessage((const char*) buf);
    recomp_free(buf);
}