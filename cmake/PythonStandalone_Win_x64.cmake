cmake_minimum_required(VERSION 3.20)
project(UsePythonStandalone)

include(FetchContent)

# URL and output directories
# set(PYTHON_URL "https://github.com/astral-sh/python-build-standalone/releases/download/20250702/cpython-3.13.5+20250702-x86_64-pc-windows-msvc-install_only_stripped.tar.gz")
set(PYTHON_URL "https://github.com/astral-sh/python-build-standalone/releases/download/20250702/cpython-3.13.5+20250702-x86_64-pc-windows-msvc-install_only.tar.gz")
set(PYTHON_ARCHIVE "${CMAKE_BINARY_DIR}/cpython.tar.gz")
set(PYTHON_EXTRACT_DIR "${CMAKE_BINARY_DIR}/python-standalone")

# Download the artifact
if(NOT EXISTS "${PYTHON_ARCHIVE}")
    message(STATUS "Downloading Python artifact...")
    file(DOWNLOAD "${PYTHON_URL}" "${PYTHON_ARCHIVE}" SHOW_PROGRESS)
endif()

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
file(GLOB EXTRACTED_DIRS LIST_DIRECTORIES true "${PYTHON_EXTRACT_DIR}/*")
list(GET EXTRACTED_DIRS 0 PYTHON_ROOT)

# Create imported interface target
add_library(python_standalone INTERFACE)
target_include_directories(python_standalone INTERFACE "${PYTHON_ROOT}/include")
target_link_directories(python_standalone INTERFACE "${PYTHON_ROOT}/libs")
target_link_libraries(python_standalone INTERFACE python313)

# When making debug builds, CMake may still look for the python lib under this name.
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PYTHON_ROOT}/libs/python313.lib" "${PYTHON_ROOT}/libs/python313_d.lib")

function(link_python_standalone TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE python_standalone)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_ROOT}/python313.dll"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/python313.dll"
    )
endfunction()

set(PYTHON_EXE "${PYTHON_ROOT}/python.exe" CACHE PATH "Python executable")
set(PYTHON_STANDALONE_ROOT "${PYTHON_ROOT}" CACHE PATH "Root of extracted Python standalone distribution")
