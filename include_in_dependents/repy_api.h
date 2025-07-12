#include "modding.h"
#include "global.h"



/*! \file repy_api.h
    \version 1.0.0
    \brief The main header for Recomp External Python
 */

#ifdef DOXYGEN
#define REPY_IMPORT(func) func

#elif RECOMP_PY_BUILD_MODE
#define REPY_IMPORT(func) func

#else 
#define EZTR_IMPORT(func) RECOMP_IMPORT(EZTR_MOD_ID_STR, func)

#endif

/**
 * @brief The mod id string for REPY.
 * 
 * The `recomp_py.h` imports all the functions and events needed for RecompPy, 
 * so you probably won't need to use this directly.
 */
#define REPY_MOD_ID_STR "MM_Recomp_External_Python_API"