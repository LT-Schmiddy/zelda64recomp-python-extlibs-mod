cmake_minimum_required(VERSION 3.20)

project(recomp_mod_native_lib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED LIB_NAME)
    set(LIB_NAME recomp_extlib)
endif()

set(TARGET_NAME ${LIB_NAME})

function(py_watch)
    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${ARGV}
    )
endfunction()

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

set(PYBIND11_PYTHON_VERSION 3.13)
set(PYTHON_EXECUTABLE ${PYTHON_EXE})
set(PYTHON_IS_DEBUG OFF)

# Adding PyBind11
set(Python_FIND_ABI "OFF" "ANY" "ANY")
set(PYBIND11_FINDPYTHON OFF)
set(PYBIND11_NOPYTHON ON)
add_subdirectory("./extern/pybind11")

# Including GraphiteMaster's Incbin:
include_directories("./extern/incbin/")

#Including PLOG:
add_subdirectory("./extern/plog/")
include_directories("./extern/plog/include")

# Establishing Build Target:
add_library(${TARGET_NAME} SHARED)
add_subdirectory("./src/extlib")
link_python_standalone(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} PRIVATE pybind11::embed)
target_link_libraries(${TARGET_NAME} PRIVATE plog::plog)
target_sources(${TARGET_NAME} PUBLIC ./extern/incbin/incbin.c)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
)
