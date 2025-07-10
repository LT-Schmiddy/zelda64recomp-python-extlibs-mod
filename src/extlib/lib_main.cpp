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

std::unordered_map<int, py::object> general_objects;
std::unordered_map<int, py::object> bytecode_objects;
std::unordered_map<int, py::dict> scope_objects;

static std::u8string cached_return_u8string;
static std::string cached_return_string;

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
    RECOMP_RETURN(int, 1);
}
// ======================================  Scopes: ======================================  
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

#define PYTHON_SCOPE_GETTER(fname, vtype) \
RECOMP_DLL_FUNC(fname) { \
    py::gil_scoped_acquire gil; \
    int scope_handle = RECOMP_ARG(int, 0); \
    std::string name = RECOMP_ARG_STR(1); \
    py::dict scope = scope_objects.at(scope_handle); \
    vtype retVal = scope[name.c_str()].cast<vtype>(); \
    RECOMP_RETURN(vtype, retVal); \
}

#define PYTHON_SCOPE_SETTER(fname, vtype) \
RECOMP_DLL_FUNC(fname) { \
    py::gil_scoped_acquire gil; \
    vtype value = RECOMP_ARG(vtype, 0); \
    int scope_handle = RECOMP_ARG(int, 1); \
    std::string name = RECOMP_ARG_STR(2); \
    py::dict scope = scope_objects.at(scope_handle); \
    scope[name.c_str()] = value; \
}

#define PYTHON_SCOPE_GETSET(fname, vtype) \
PYTHON_SCOPE_GETTER(PythonNative_Scope_Get ## fname, vtype); \
PYTHON_SCOPE_SETTER(PythonNative_Scope_Set ## fname, vtype); \

PYTHON_SCOPE_GETSET(Bool, bool);
PYTHON_SCOPE_GETSET(U32, unsigned int);
PYTHON_SCOPE_GETSET(S32, int);
PYTHON_SCOPE_GETSET(F32, float);

RECOMP_DLL_FUNC(PythonNative_Scope_SetString) {
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);
    int scope_handle = RECOMP_ARG(int, 1);
    std::string name = RECOMP_ARG_STR(2);
    py::dict scope = scope_objects.at(scope_handle);
    scope[name.c_str()] = py::str(value);
}

RECOMP_DLL_FUNC(PythonNative_Scope_SetStringN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);
    int scope_handle = RECOMP_ARG(int, 2);
    std::string name = RECOMP_ARG_STR(3);
    py::dict scope = scope_objects.at(scope_handle);
    scope[name.c_str()] = py::str(value);
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetString_Prepare) {
    py::gil_scoped_acquire gil;
    int scope_handle = RECOMP_ARG(int, 0);
    std::string name = RECOMP_ARG_STR(1);
    py::dict scope = scope_objects.at(scope_handle);
    cached_return_u8string = scope[name.c_str()].cast<std::u8string>();

    RECOMP_RETURN(unsigned int, cached_return_u8string.size());
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetString_Copy) {
    py::gil_scoped_acquire gil;
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_u8string.at(i);
    }
}

RECOMP_DLL_FUNC(PythonNative_Scope_SetBytes) {
    py::gil_scoped_acquire gil;
    std::string value = RECOMP_ARG_STR(0);
    int scope_handle = RECOMP_ARG(int, 1);
    std::string name = RECOMP_ARG_STR(2);
    py::dict scope = scope_objects.at(scope_handle);
    scope[name.c_str()] = py::bytes(value);
}

RECOMP_DLL_FUNC(PythonNative_Scope_SetBytesN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);
    int scope_handle = RECOMP_ARG(int, 2);
    std::string name = RECOMP_ARG_STR(3);
    py::dict scope = scope_objects.at(scope_handle);
    scope[name.c_str()] = py::bytes(value);
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetBytes_Prepare) {
    py::gil_scoped_acquire gil;
    int scope_handle = RECOMP_ARG(int, 0);
    std::string name = RECOMP_ARG_STR(1);
    py::dict scope = scope_objects.at(scope_handle);
    cached_return_string = scope[name.c_str()].cast<std::string>();

    RECOMP_RETURN(unsigned int, cached_return_string.size());
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetBytes_Copy) {
    py::gil_scoped_acquire gil;
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_string.at(i);
    }
}

// ====================================== Execution: ====================================== 
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

RECOMP_DLL_FUNC(PythonNative_CompileBytecodeN) {
    py::gil_scoped_acquire gil;
    unsigned int code_len = RECOMP_ARG(unsigned int, 1);
    std::string code_str = RECOMP_ARG_STR_N(0, code_len);
    std::string identifier = RECOMP_ARG_STR(2);
    unsigned int code_type = RECOMP_ARG(unsigned int, 3);

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

RECOMP_DLL_FUNC(PythonNative_Execute) {
    py::gil_scoped_acquire gil;
    int code_handle = RECOMP_ARG(int, 0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::object bytecode = bytecode_objects.at(code_handle);
    py::dict scope = scope_objects.at(scope_handle);

    try {
        py_exec(bytecode, scope);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(int, 0);
    }
    RECOMP_RETURN(int, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecuteString) {
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::dict scope = scope_objects.at(scope_handle);

    try {
        py_exec(code_string, scope);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(int, 0);
    }

    RECOMP_RETURN(int, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecuteStringN) {
    py::gil_scoped_acquire gil;
    unsigned int code_len = RECOMP_ARG(unsigned int, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    int scope_handle = RECOMP_ARG(int, 1);

    py::dict scope = scope_objects.at(scope_handle);

    try {
        py_exec(code_string, scope);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(int, 0);
    }

    RECOMP_RETURN(int, 1);
}

RECOMP_DLL_FUNC(PythonNative_LoadModule) {
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);
    std::string code_string = RECOMP_ARG_STR(1);

    embedded_import::construct_module(module_name, code_string, true);
}

RECOMP_DLL_FUNC(PythonNative_LoadModuleN) {
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);
    unsigned int code_len = RECOMP_ARG(unsigned int, 2);
    std::string code_string = RECOMP_ARG_STR_N(1, code_len);

    embedded_import::construct_module(module_name, code_string, true);
}
