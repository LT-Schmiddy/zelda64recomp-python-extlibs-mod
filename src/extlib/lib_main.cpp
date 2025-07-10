#include <chrono>
#include <random>
#include <unordered_map>

#include "lib_recomp.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include "embedded_import.hpp"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

static const char* code_type_strs[] = {
    "exec",
    "eval",
    "single"
};

static PyThreadState* py_main_thread = NULL;

std::unordered_map<int, py::object> bytecode_objects;
std::unordered_map<int, py::dict> scope_objects;


int get_new_handle() {
    return random_in_range(1, INT_MAX);
}

RECOMP_DLL_FUNC(PythonNative_Init) {
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(0);
    fs::path mod_dir(mod_dir_text);
    fs::path mod_dir_DLLs = fs::path(mod_dir).append("DLLs");
    fs::path mod_dir_Lib = fs::path(mod_dir).append("Lib");

    printf("Mod Folder: %s\n", (char*)mod_dir_text.c_str());

    // start the interpreter and keep it alive
    py::initialize_interpreter(); 

    {
        py::gil_scoped_acquire gil;
        // Setting the module search path for the interpreter
        auto sys = py::module_::import("sys");
        auto sys_path = sys.attr("path");
        sys_path.attr("clear")();
        sys_path.attr("append")(mod_dir.string());
        sys_path.attr("append")(mod_dir_DLLs.string());
        sys_path.attr("append")(mod_dir_Lib.string());
    }

    collect_py_functions();
    // Release the GIL so that Python threads can run in the background.
    // This does mean that all other functions that operate on python will need to reaquire the GIL.
    py_main_thread = PyEval_SaveThread();
    RECOMP_RETURN(int, 0);
}

RECOMP_DLL_FUNC(PythonNative_CompileBytecode) {
    py::gil_scoped_acquire gil;
    std::string code_str = RECOMP_ARG_STR(0);
    std::string identifier = RECOMP_ARG_STR(1);
    unsigned int code_type = RECOMP_ARG(unsigned int, 2);

    py::object bytecode;
    try {
        bytecode = py_compile(code_str, identifier, code_type_strs[code_type]);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(int, 0);
    }

    int new_handle = 0;
    while (bytecode_objects.contains(new_handle) || new_handle == 0) {
        new_handle = get_new_handle();
    }

    bytecode_objects.insert({new_handle, bytecode});

    RECOMP_RETURN(int, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_ReleaseBytecode) {
    py::gil_scoped_acquire gil;
    int handle = RECOMP_ARG(int, 0);

    bytecode_objects.erase(handle);
}

RECOMP_DLL_FUNC(PythonNative_CreateScope) {
    py::gil_scoped_acquire gil;

    py::dict new_dict = py::dict();

    int new_handle = 0;
    while (scope_objects.contains(new_handle) || new_handle == 0) {
        new_handle = get_new_handle();
    }

    scope_objects.insert({new_handle, new_dict});

    RECOMP_RETURN(int, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_ReleaseScope) {
    py::gil_scoped_acquire gil;
    int handle = RECOMP_ARG(int, 0);

    scope_objects.erase(handle);
}

RECOMP_DLL_FUNC(PythonNative_Execute) {
    py::gil_scoped_acquire gil;
    int code_handle = RECOMP_ARG(int, 0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::object bytecode = bytecode_objects.at(code_handle);
    py::object scope = scope_objects.at(scope_handle);

    try {
        py_exec(bytecode, scope);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
    }
}

RECOMP_DLL_FUNC(PythonNative_ExecuteString) {
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::object scope = scope_objects.at(scope_handle);

    try {
        py_exec(code_string, scope);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
    }
}

// Type Getters
