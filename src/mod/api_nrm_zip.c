#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "printf.h"
#include "extlib_functions.h"
#include "mod_logging.h"

RECOMP_EXPORT REPY_Handle REPY_GetNrmZipForFile(const char* filepath) {
    return PythonNative_GetZipFile_CStr(filepath);
}