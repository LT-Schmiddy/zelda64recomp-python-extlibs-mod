cmake_minimum_required(VERSION 3.20)
project(UsePythonStandalone)

include(FetchContent)

# URL and output directories
set(PYTHON_URL "https://github.com/astral-sh/python-build-standalone/releases/download/20250702/cpython-3.13.5+20250702-x86_64-unknown-linux-gnu-install_only_stripped.tar.gz")
# set(PYTHON_URL "https://github.com/astral-sh/python-build-standalone/releases/download/20250702/cpython-3.13.5+20250702-x86_64-unknown-linux-gnu-install_only.tar.gz")
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
target_include_directories(python_standalone INTERFACE "${PYTHON_ROOT}/include/python3.13")
target_link_directories(python_standalone INTERFACE "${PYTHON_ROOT}/lib")
target_link_libraries(python_standalone INTERFACE libpython3.13.so.1.0)

## Seems to be a problem linking if we don't remove the `1.0` from the library extension.
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PYTHON_ROOT}/lib/libpython3.13.so.1.0" "${PYTHON_ROOT}/lib/libpython3.13.so")


function(link_python_standalone TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE python_standalone)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_ROOT}/lib/libpython3.13.so.1.0"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/libpython3.13.so"
    )
endfunction()

set(PYTHON_EXE "${PYTHON_ROOT}/bin/python3.13" CACHE PATH "Python executable")
set(PYTHON_STANDALONE_ROOT "${PYTHON_ROOT}" CACHE PATH "Root of extracted Python standalone distribution")

# Create Standard Library Archive:
set(PYTHON_INCBIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/inbin/" CACHE PATH "Any data file that needs to be included in the binary will be generated here.")
make_directory("${PYTHON_INCBIN_DIR}")
include_directories("${PYTHON_INCBIN_DIR}")

file(GLOB_RECURSE PYTHON_STDLIB "${PYTHON_STANDALONE_ROOT}/lib/python3.13/**")
file(ARCHIVE_CREATE 
    OUTPUT "${PYTHON_INCBIN_DIR}/python313_stdlib.zip" 
    PATHS ${PYTHON_STDLIB}
    WORKING_DIRECTORY "${PYTHON_STANDALONE_ROOT}/lib/python3.13"
    FORMAT "zip"

)