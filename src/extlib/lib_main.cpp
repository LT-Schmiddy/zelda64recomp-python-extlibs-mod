#include <chrono>
#include <random>
#include <unordered_map>

#include "lib_main.hpp"
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

std::unordered_map<int, PyObjectHandleEntry> py_objects;

static std::u8string cached_return_u8string;
static std::string cached_return_string;


// ======================================  Handle Control: ====================================== 
PyObjectHandle get_new_handle_value() {
    PyObjectHandle new_handle = 0;
    while (py_objects.contains(new_handle) || new_handle == 0) {
        new_handle = random_in_range(1, INT_MAX);
    }
    return new_handle;
}

int create_py_handle(py::object obj) {
    PyObjectHandle new_handle = get_new_handle_value();
    py_objects.insert({new_handle, {obj, false}});
    return new_handle;
}

py::object get_py_object(PyObjectHandle handle) {
    
    PyObjectHandleEntry* entry = &py_objects.at(handle);
    py::object retVal = entry->py_object;
    if (entry->is_single_use) {
        py_objects.erase(handle);
    }
    return retVal;
}

// ======================================  API INIT: ====================================== 
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
// ======================================  General: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Object_Release) {
    py::gil_scoped_acquire gil;
    int handle = RECOMP_ARG(int, 0);

    py_objects.erase(handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_MakeSUH) {
    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    py_objects.at(handle).is_single_use = true;

    RECOMP_RETURN(int, handle);
}

// ======================================  Modules: ====================================== 
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

// ======================================  Casting: ======================================  
#define PYTHON_OBJECT_CREATE(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    py::gil_scoped_acquire gil; \
    c_type value = RECOMP_ARG(c_type, 0); \
    py::object obj = py_type(value); \
    int new_handle = create_py_handle(py_type(value)); \
    RECOMP_RETURN(int, new_handle); \
}

#define PYTHON_OBJECT_CAST(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    py::gil_scoped_acquire gil; \
    py_type obj = RECOMP_ARG_PYOBJECT(0); \
    c_type retVal = obj.cast<c_type>(); \
    RECOMP_RETURN(c_type, retVal); \
}

#define PYTHON_OBJECT_CREATECAST(fname, c_type, py_type) \
PYTHON_OBJECT_CREATE(PythonNative_Object_Create ## fname, c_type, py_type); \
PYTHON_OBJECT_CAST(PythonNative_Object_Cast ## fname, c_type, py_type); \

PYTHON_OBJECT_CREATECAST(Bool, unsigned int, py::bool_);
PYTHON_OBJECT_CREATECAST(U32, unsigned int, py::int_);
PYTHON_OBJECT_CREATECAST(S32, int, py::int_);
PYTHON_OBJECT_CREATECAST(F32, float, py::float_);

RECOMP_DLL_FUNC(PythonNative_Object_CreateStr) {
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::str obj = py::str(value);
    PyObjectHandle retVal = create_py_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateStrN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);
    PyObjectHandle scope_handle = RECOMP_ARG(int, 2);

    py::str obj = py::str(value);
    PyObjectHandle retVal = create_py_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CastStr_Prepare) {
    py::gil_scoped_acquire gil;
    py::str str = RECOMP_ARG_PYOBJECT(0);
    cached_return_u8string = str.cast<std::u8string>();
    RECOMP_RETURN(unsigned int, cached_return_u8string.size());
}

RECOMP_DLL_FUNC(PythonNative_Object_CastStr_Copy) {
    // Don't need the GIL for this step.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_u8string.at(i);
    }
}

RECOMP_DLL_FUNC(PythonNative_Scope_CreateBytes) {
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);
    int scope_handle = RECOMP_ARG(int, 1);

    py::str obj = py::str(value);
    PyObjectHandle retVal = create_py_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Scope_CreateBytesN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);
    PyObjectHandle scope_handle = RECOMP_ARG(int, 2);

    py::bytes obj = py::bytes(value);
    PyObjectHandle retVal = create_py_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetBytes_Prepare) {
    py::gil_scoped_acquire gil;
    py::str str = RECOMP_ARG_PYOBJECT(0);
    cached_return_string = str.cast<std::string>();
    RECOMP_RETURN(unsigned int, cached_return_string.size());
}

RECOMP_DLL_FUNC(PythonNative_Scope_GetBytes_Copy) {
    // Don't actually need the GIL for this one.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_string.at(i);
    }
}

// ======================================  Dicts: ======================================  
RECOMP_DLL_FUNC(PythonNative_Dict_Create) {
    py::gil_scoped_acquire gil;

    PyObjectHandle new_handle = create_py_handle(py::dict());

    RECOMP_RETURN(PyObjectHandle, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Get) {
    py::gil_scoped_acquire gil;
    py::dict d = RECOMP_ARG_PYOBJECT(0);
    py::object key = RECOMP_ARG_PYOBJECT(1);

    py::object obj = d[key];

    PyObjectHandle retVal = create_py_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Set) {
    py::gil_scoped_acquire gil;
    py::dict d = RECOMP_ARG_PYOBJECT(0);
    py::object key = RECOMP_ARG_PYOBJECT(1);
    py::object value = RECOMP_ARG_PYOBJECT(2);

    d[key] = value;
}

RECOMP_DLL_FUNC(PythonNative_Dict_Has) {
    py::gil_scoped_acquire gil;
    py::dict d = RECOMP_ARG_PYOBJECT(0);
    py::object key = RECOMP_ARG_PYOBJECT(1);

    unsigned int retVal = d.contains(key);
    RECOMP_RETURN(unsigned int, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Remove) {
    py::gil_scoped_acquire gil;
    py::dict d = RECOMP_ARG_PYOBJECT(0);
    py::object key = RECOMP_ARG_PYOBJECT(1);

    d.attr("pop")(key);
}


// ====================================== Execution: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Compile) {
    py::gil_scoped_acquire gil;
    py::str code_str = RECOMP_ARG_PYOBJECT(0);
    py::str identifier_str = RECOMP_ARG_PYOBJECT(1);
    py::str type_str = RECOMP_ARG_PYOBJECT(2);

    py::object bytecode;
    try {
        bytecode = py_compile(code_str, identifier_str, type_str);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(int, 0);
    }

    PyObjectHandle handle = create_py_handle(bytecode);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CompileCStr) {
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

    PyObjectHandle handle = create_py_handle(bytecode);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CompileCStrN) {
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

    PyObjectHandle handle = create_py_handle(bytecode);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Exec) {
    py::gil_scoped_acquire gil;
    py::object bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict globals = RECOMP_ARG_PYOBJECT(1);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(2);
    }

    try {
        py_exec(bytecode, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(unsigned int, 0);
    }
    RECOMP_RETURN(unsigned int, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStr) {
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict globals = RECOMP_ARG_PYOBJECT(1);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(2);
    }

    try {
        py_exec(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(unsigned int, 0);
    }
    RECOMP_RETURN(unsigned int, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStrN) {
    py::gil_scoped_acquire gil;
    unsigned int code_len = RECOMP_ARG(unsigned int, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict globals = RECOMP_ARG_PYOBJECT(2);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 3) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(3);
    }

    try {
        py_exec(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(unsigned int, 0);
    }
    RECOMP_RETURN(unsigned int, 1);
}

RECOMP_DLL_FUNC(PythonNative_Eval) {
    py::gil_scoped_acquire gil;
    py::object bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict globals = RECOMP_ARG_PYOBJECT(1);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(2);
    }
    
    py::object result;
    try {
        result = py_eval(bytecode, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = create_py_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStr) {
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict globals = RECOMP_ARG_PYOBJECT(1);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(2);
    }

    py::object result;
    try {
        result = py_eval(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = create_py_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStrN) {
    py::gil_scoped_acquire gil;
    unsigned int code_len = RECOMP_ARG(unsigned int, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict globals = RECOMP_ARG_PYOBJECT(2);
    py::dict locals;
    if (RECOMP_ARG(PyObjectHandle, 3) == 0) {
        locals = globals;
    } else {
        locals = RECOMP_ARG_PYOBJECT(3);
    }

    py::object result;
    try {
        result = py_eval(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = create_py_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}