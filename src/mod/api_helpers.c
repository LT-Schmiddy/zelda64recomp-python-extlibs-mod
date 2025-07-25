#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recomp_incbin.h"
#include "libc/stdarg.h"

#include "extlib_functions.h"

RECOMP_EXPORT u32 REPY_CompileHelper (
    REPY_Handle* handle_ptr, 
    const char* code_str, 
    const char* identifier,
    REPY_CodeMode code_mode, 
    REPY_CHReturnType return_type
) {
    recomp_printf("Running REPY_CompileHelper\n");
    bool was_compiled = false;
    if (*handle_ptr == 0) {
        *handle_ptr = REPY_CompileCStr(code_str, identifier, code_mode);
        recomp_printf("REPY_CompileHelper compiled handle %u\n", *handle_ptr);
        bool was_compiled = true;
    } 

    // Handling Return:
    switch(return_type) {
        case REPY_RETURN_TRUE:
            return true;
        case REPY_RETURN_WAS_COMPILED:
            return was_compiled;
        case REPY_RETURN_WAS_COMPILED_SUCCESSFULLY:
            return was_compiled && *handle_ptr;
        case REPY_RETURN_HANDLE:
            return *handle_ptr;
        case REPY_RETURN_FALSE:
        default:
            return false;
    }
}

RECOMP_EXPORT REPY_IteratorHelper REPY_IteratorHelper_Init() {
    
}