cmake_minimum_required(VERSION 3.31)
project(UsePythonStandalone)

set(PYTHON_LIB_VERSION_STR "$ENV{PYTHON_ARCHIVE_VSTR_POSIX}")

# Get the artifact downloaded by Python
set(PYTHON_ARCHIVE "$ENV{PYTHON_LINUX_ARCHIVE}")
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
set(PYTHON_ROOT "${PYTHON_EXTRACT_DIR}/python/install")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so.1.0" "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so")

# Create imported interface target
add_library(python_standalone INTERFACE)
target_include_directories(python_standalone INTERFACE "${PYTHON_ROOT}/include/${PYTHON_LIB_VERSION_STR}")
target_link_directories(python_standalone INTERFACE "${PYTHON_ROOT}/lib")
target_link_libraries(python_standalone INTERFACE lib${PYTHON_LIB_VERSION_STR}.so.1.0)
# target_link_libraries(python_standalone INTERFACE lib${PYTHON_LIB_VERSION_STR}.so)

## Seems to be a problem linking if we don't remove the `1.0` from the library extension.
# execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so.1.0" "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so")

function(link_python_standalone TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE python_standalone)

    # add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #             "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so.1.0"
    #             "$<TARGET_FILE_DIR:${TARGET_NAME}>/lib${PYTHON_LIB_VERSION_STR}.so"
    # )
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.so.1.0"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/lib${PYTHON_LIB_VERSION_STR}.so.1.0"
    )
endfunction()

set(PYTHON_EXE "${PYTHON_ROOT}/bin/${PYTHON_LIB_VERSION_STR}" CACHE PATH "Python executable")
set(PYTHON_STANDALONE_ROOT "${PYTHON_ROOT}" CACHE PATH "Root of extracted Python standalone distribution")

# Create Standard Library Archive:
set(PYTHON_INCBIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/incbin/" CACHE PATH "Any data file that needs to be included in the binary will be generated here.")
make_directory("${PYTHON_INCBIN_DIR}")
include_directories("${PYTHON_INCBIN_DIR}")

file(GLOB_RECURSE PYTHON_STDLIB "${PYTHON_STANDALONE_ROOT}/lib/${PYTHON_LIB_VERSION_STR}/**")
file(ARCHIVE_CREATE 
    OUTPUT "${PYTHON_INCBIN_DIR}/python_stdlib.zip" 
    PATHS ${PYTHON_STDLIB}
    WORKING_DIRECTORY "${PYTHON_STANDALONE_ROOT}/lib/${PYTHON_LIB_VERSION_STR}"
    FORMAT "zip"

)