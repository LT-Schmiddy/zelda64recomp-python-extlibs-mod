cmake_minimum_required(VERSION 3.20)

project(recomp_mod_native_lib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME ${LIB_NAME})

include_directories("./offline_build")
include_directories("./include/extlib")
include_directories("./include/shared")
include_directories("./src/extlib")

# Include the script to download and set up the Python binary
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_Win_x64.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # Macos
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_MacOS_aarch64.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # 64-bit Linux
    include(${CMAKE_SOURCE_DIR}/cmake/PythonStandalone_Linux_x64.cmake)
endif()

add_library(${TARGET_NAME} SHARED)

add_subdirectory("./src/extlib")

link_python_standalone(${TARGET_NAME})
set_target_properties(${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
)
