cmake_minimum_required(VERSION 3.31)
project(UsePythonStandalone)

set(PYTHON_LIB_VERSION_STR "$ENV{PYTHON_ARCHIVE_VSTR_NT}")

# Get the artifact downloaded by Python
set(PYTHON_ARCHIVE "$ENV{PYTHON_WIN_ARCHIVE}")
set(PYTHON_EXTRACT_DIR "${CMAKE_BINARY_DIR}/python-standalone")

# Extract it
if(NOT EXISTS "${PYTHON_EXTRACT_DIR}")
    message(STATUS "Extracting Python artifact...")
    file(MAKE_DIRECTORY "${PYTHON_EXTRACT_DIR}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${PYTHON_ARCHIVE}"
        WORKING_DIRECTORY "${PYTHON_EXTRACT_DIR}"
    )
endif()

# Find the real root of the extracted directory (the archive contains a folder)
set(PYTHON_ROOT "${PYTHON_EXTRACT_DIR}/python")

# Create imported interface target
add_library(python_standalone INTERFACE)
target_include_directories(python_standalone INTERFACE "${PYTHON_ROOT}/include")
target_link_directories(python_standalone INTERFACE "${PYTHON_ROOT}/libs")
target_link_libraries(python_standalone INTERFACE ${PYTHON_LIB_VERSION_STR})

# When making debug builds, CMake may still look for the python lib under this name.
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PYTHON_ROOT}/libs/${PYTHON_LIB_VERSION_STR}.lib" "${PYTHON_ROOT}/libs/${PYTHON_LIB_VERSION_STR}_d.lib")

function(link_python_standalone TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE python_standalone)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_ROOT}/${PYTHON_LIB_VERSION_STR}.dll"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/${PYTHON_LIB_VERSION_STR}.dll"
    )
endfunction()

set(PYTHON_EXE "${PYTHON_ROOT}/python.exe" CACHE PATH "Python executable")
set(PYTHON_STANDALONE_ROOT "${PYTHON_ROOT}" CACHE PATH "Root of extracted Python standalone distribution")

# Create Standard Library Archive:
set(PYTHON_INCBIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/incbin/" CACHE PATH "Any data file that needs to be included in the binary will be generated here.")
make_directory("${PYTHON_INCBIN_DIR}")
include_directories("${PYTHON_INCBIN_DIR}")

file(GLOB_RECURSE PYTHON_STDLIB "${PYTHON_STANDALONE_ROOT}/Lib/**")
file(ARCHIVE_CREATE 
    OUTPUT "${PYTHON_INCBIN_DIR}/python_stdlib.zip" 
    PATHS ${PYTHON_STDLIB}
    WORKING_DIRECTORY "${PYTHON_STANDALONE_ROOT}/Lib"
    FORMAT "zip"
)