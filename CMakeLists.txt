cmake_minimum_required(VERSION 3.20)

project(recomp_mod_native_lib)

# Include the script to download and set up the Python binary
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_Win_x64.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # Macos
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_MacOS_aarch64.cmake)
    set(CMAKE_MACOSX_RPATH On)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # 64-bit Linux
    set(CMAKE_BUILD_RPATH_USE_ORIGIN On)
    set(CMAKE_BUILD_RPATH "$ORIGIN/.")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/.")
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_Linux_x64.cmake)
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED ENV{LIB_NAME})
    set(TARGET_NAME recomp_extlib)
else()
    set(TARGET_NAME $ENV{LIB_NAME})
endif()

include_directories("./offline_build")
include_directories("./include/extlib")
include_directories("./include/shared")
include_directories("./src/extlib")

## Setting up Python building:
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit
    message(STATUS "Building for 64-bit...")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32-bit
    message(FATAL_ERROR "Building for 32-bit not supported.")
else()
    message(FATAL_ERROR "Unknown pointer size.")
endif()


set(PYBIND11_PYTHON_VERSION 3.14t)
set(PYTHON_EXECUTABLE ${PYTHON_EXE})
set(PYTHON_IS_DEBUG OFF)

# Adding PyBind11
set(Python_FIND_ABI "OFF" "ANY" "ANY")
set(PYBIND11_FINDPYTHON OFF)
set(PYBIND11_NOPYTHON ON)
add_subdirectory("./extern/pybind11")

# Including GraphiteMaster's Incbin:
include_directories("./extern/incbin/")

# Including PLOG:
add_subdirectory("./extern/plog/")
include_directories("./extern/plog/include")

option (TRACY_ENABLE "" ON)
option (TRACY_ON_DEMAND "" OFF)
# option (TRACY_ONLY_LOCALHOST "" ON)

include_directories("./extern/tracy/public")

# Establishing Build Target:
add_library(${TARGET_NAME} SHARED)
add_subdirectory("./src/extlib")
target_link_libraries(${TARGET_NAME} PRIVATE pybind11::embed)
target_link_libraries(${TARGET_NAME} PRIVATE plog::plog)
link_python_standalone(${TARGET_NAME})

if(DEFINED TRACY_PROFILING)
    message(STATUS "Tracy Enabled")
    add_subdirectory("./extern/tracy/")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTRACY_ENABLE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRACY_ENABLE")
    target_link_libraries(${TARGET_NAME} PRIVATE Tracy::TracyClient)
    # target_sources(${TARGET_NAME} PUBLIC "./extern/tracy/public/TracyClient.cpp")
endif()

set_target_properties(${TARGET_NAME}
    PROPERTIES
    PREFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
)

# Defining Native Testing Target:

set(TEST_NAME "NativeTest_${CMAKE_SYSTEM_NAME}")
add_executable(${TEST_NAME})
add_subdirectory("./src/extlib_tests")

set_target_properties(${TEST_NAME}
    PROPERTIES
    PREFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
)



