cmake_minimum_required(VERSION 3.31)
project(UsePythonStandalone)

set(PYTHON_LIB_VERSION_STR "$ENV{PYTHON_ARCHIVE_VSTR_POSIX}")

# Get the artifact downloaded by Python
set(PYTHON_ARCHIVE "$ENV{PYTHON_MACOS_ARCHIVE}")
set(PYTHON_EXTRACT_DIR "${CMAKE_BINARY_DIR}/python-standalone")

# Handling the install_name change for the dylib:
if (CMAKE_HOST_SYSTEM MATCHES "Darwin")
    set(INSTALL_NAME_TOOL_COMMAND llvm-install-name-tool)
elseif(CMAKE_HOST_SYSTEM MATCHES "Windows")
    set(INSTALL_NAME_TOOL_COMMAND llvm-install-name-tool)
elseif(CMAKE_HOST_SYSTEM MATCHES "Linux")
    set(INSTALL_NAME_TOOL_COMMAND llvm-install-name-tool)
endif()

# Extract the artifact
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

function(log_target_property P_TARGET_NAME P_PROP_NAME)
    get_target_property(PRINT_VAR ${P_TARGET_NAME} ${P_PROP_NAME})
    message(STATUS "Property '${P_PROP_NAME}' of Target '${P_TARGET_NAME}' = ${PRINT_VAR}")

endfunction()

# Create imported interface target
add_library(python_standalone INTERFACE)
target_include_directories(python_standalone INTERFACE "${PYTHON_ROOT}/include/${PYTHON_LIB_VERSION_STR}")
target_link_directories(python_standalone INTERFACE "${PYTHON_ROOT}/lib")
target_link_libraries(python_standalone INTERFACE lib${PYTHON_LIB_VERSION_STR}.dylib)

function(link_python_standalone P_TARGET_NAME)
    string(CONCAT DYLIB_FILE $<TARGET_FILE_DIR:${P_TARGET_NAME}> "/lib" ${P_TARGET_NAME} ".dylib")

    target_link_libraries(${P_TARGET_NAME} PRIVATE python_standalone)
    add_custom_command(TARGET ${P_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_ROOT}/lib/lib${PYTHON_LIB_VERSION_STR}.dylib"
                "$<TARGET_FILE_DIR:${P_TARGET_NAME}>/lib${PYTHON_LIB_VERSION_STR}.dylib"
    )

    message("CMAKE_HOST_SYSTEM = ${CMAKE_HOST_SYSTEM}")
    message("INSTALL_NAME_TOOL_COMMAND = ${INSTALL_NAME_TOOL_COMMAND}")

    add_custom_command(TARGET ${P_TARGET_NAME} POST_BUILD
        COMMAND ${INSTALL_NAME_TOOL_COMMAND} -change /install/lib/lib${PYTHON_LIB_VERSION_STR}.dylib @loader_path/lib${PYTHON_LIB_VERSION_STR}.dylib ${DYLIB_FILE}
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