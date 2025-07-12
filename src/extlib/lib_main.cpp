#include <chrono>
#include <random>
#include <unordered_map>
#include <plog/Log.h> // Step1: include the headers
#include <plog/Initializers/RollingFileInitializer.h>

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

static std::u8string cached_return_u8string;
static std::string cached_return_string;

// ======================================  Handle Control: ====================================== 

PyInterpreterController::PyInterpreterController() {
    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);
    Py_PreInitialize(&preconfig);

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.parse_argv = 0;
    config.install_signal_handlers = true;

    py::initialize_interpreter(&config); 
    PLOGD << "-> Python Interpreter Parent: INIT\n";
    // Allow other threads to have the GIL.
    py_main_thread = PyEval_SaveThread();
};

PyInterpreterController::~PyInterpreterController() {
    // Restores the GIL to this thread.

    // The DLL unloading process seems to clean up the interpreter on it's own,
    // But it doesn't seem to like it when we have handles left over.
    {
        py::gil_scoped_acquire gil;
        py_objects.clear();
    }
    PyEval_RestoreThread(py_main_thread);
    PLOGD << "-> Python Interpreter Parent: DEINIT\n";
}

PyObjectHandle PyInterpreterController::get_new_handle_value() {
    py::gil_scoped_acquire gil;
    PyObjectHandle new_handle = 0;
    while (py_objects.contains(new_handle) || new_handle == 0) {
        new_handle = random_in_range(1, INT_MAX);
    }
    return new_handle;
}

int PyInterpreterController::create_handle(py::object obj) {
    py::gil_scoped_acquire gil;
    PyObjectHandle new_handle = get_new_handle_value();
    py_objects.insert({new_handle, {obj, false}});

    PLOGD.printf("-> PyObjectHandle %i Created\n", new_handle);
    return new_handle;
}

py::object PyInterpreterController::get_py_object(PyObjectHandle handle) {
    PyObjectHandleEntry* entry = &py_objects.at(handle);
    py::object retVal = entry->py_object;
    if (entry->is_single_use) {
        py_objects.erase(handle);
        PLOGD.printf("-> PyObjectHandle %i Accessed and Released (SUH)\n", handle);
    } else {
        PLOGD.printf("-> PyObjectHandle %i Accessed\n", handle);
    }
    return retVal;
}

bool PyInterpreterController::get_handle_suh(PyObjectHandle handle) {
    PyObjectHandleEntry* entry = &py_objects.at(handle);
    return entry->is_single_use;
}

void PyInterpreterController::set_handle_suh(PyObjectHandle handle, bool is_single_use) {
    PyObjectHandleEntry* entry = &py_objects.at(handle);
    entry->is_single_use = is_single_use;
    PLOGD.printf("-> PyObjectHandle %i Setting SUH = %i\n", handle, is_single_use);
}


void PyInterpreterController::release_handle(PyObjectHandle handle) {
    py_objects.erase(handle);
    PLOGD.printf("-> PyObjectHandle %i Released\n", handle);
}

std::shared_ptr<PyInterpreterController> controller = NULL;

// ======================================  API INIT: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Init) {
    unsigned int log_level = RECOMP_ARG(unsigned int, 0);
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(1);
    fs::path mod_dir(mod_dir_text);
    fs::path py_log_file = fs::path(mod_dir).append("REPY.log");
    plog::init(plog::debug, py_log_file.c_str());

    fs::path mod_dir_DLLs = fs::path(mod_dir).append("DLLs");
    fs::path mod_dir_Lib = fs::path(mod_dir).append("Lib");
    fs::path mod_dir_site = fs::path(mod_dir_Lib).append("site-packages");

    printf("Mod Folder: %s\n", (char*)mod_dir_text.c_str());
    plog::init((plog::Severity)log_level, "Hello.txt");



    controller = std::make_shared<PyInterpreterController>();
    {
        py::gil_scoped_acquire gil;
        // Setting the module search path for the interpreter
        auto sys = py::module_::import("sys");
        auto sys_path = sys.attr("path");
        py::print(sys_path);
        sys_path.attr("clear")();
        sys_path.attr("append")(mod_dir.string());
        sys_path.attr("append")(mod_dir_DLLs.string());
        sys_path.attr("append")(mod_dir_Lib.string());
        sys_path.attr("append")(mod_dir_site.string());
    }

    collect_py_functions();
    // Release the GIL so that Python threads can run in the background.
    // This does mean that all other functions that operate on python will need to reaquire the GIL.
    RECOMP_RETURN(int, 1);
}
// ======================================  General: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Object_Release) {
    py::gil_scoped_acquire gil;
    int handle = RECOMP_ARG(int, 0);

    controller->release_handle(handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_MakeSUH) {
    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    controller->set_handle_suh(handle, true);

    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_GetSUH) {
    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    RECOMP_RETURN(unsigned int, (unsigned int) controller->get_handle_suh(handle));
}


RECOMP_DLL_FUNC(PythonNative_Object_SetSUH) {
    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);
    PyObjectHandle value = RECOMP_ARG(unsigned int, 1);

    controller->set_handle_suh(handle, value);
}


RECOMP_DLL_FUNC(PythonNative_Object_CopyHandle) {
    py::gil_scoped_acquire gil;
    py::object object = RECOMP_ARG_PYOBJECT(0);
    PyObjectHandle new_handle = controller->create_handle(object);

    RECOMP_RETURN(PyObjectHandle, new_handle);
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

RECOMP_DLL_FUNC(PythonNative_ImportModule) {
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);

    PyObjectHandle handle = controller->create_handle(py::module_::import(module_name.c_str()));
    RECOMP_RETURN(PyObjectHandle, handle);
}

// ====================================== Primative Casting: ======================================  
#define PYTHON_OBJECT_CREATE(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    py::gil_scoped_acquire gil; \
    c_type value = RECOMP_ARG(c_type, 0); \
    py::object obj = py_type(value); \
    int new_handle = controller->create_handle(py_type(value)); \
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


// ======================================  String Casting: ======================================
RECOMP_DLL_FUNC(PythonNative_Object_CreateStr) {
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateStrN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(obj);
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

RECOMP_DLL_FUNC(PythonNative_Object_CreateBytes) {
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateBytesN) {
    py::gil_scoped_acquire gil;
    unsigned int str_len = RECOMP_ARG(unsigned int, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);

    py::bytes obj = py::bytes(value);
    PyObjectHandle retVal = controller->create_handle(obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CastBytes_Prepare) {
    py::gil_scoped_acquire gil;
    py::str str = RECOMP_ARG_PYOBJECT(0);
    cached_return_string = str.cast<std::string>();
    RECOMP_RETURN(unsigned int, cached_return_string.size());
}

RECOMP_DLL_FUNC(PythonNative_Object_CastBytes_Copy) {
    // Don't actually need the GIL for this one.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_string.at(i);
    }
}
// ======================================  Tuple: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Tuple_Create) {
    py::gil_scoped_acquire gil;
    unsigned int size = RECOMP_ARG(unsigned int, 0);
    PyObjectHandle* va_args_ptr = RECOMP_ARG(PyObjectHandle*, 1);
    
    py::list tmp = py::list();
    for (int i = 0; i < size; i++) {
        tmp.append(controller->get_py_object(va_args_ptr[i]));
    }

    PyObjectHandle handle = controller->create_handle(py::tuple(tmp));
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Tuple_GetMember) {
    py::gil_scoped_acquire gil;
    py::tuple tuple = RECOMP_ARG_PYOBJECT(0); 
    int index = RECOMP_ARG(int, 1); 

    PyObjectHandle handle = controller->create_handle(tuple[index]);
    RECOMP_RETURN(PyObjectHandle, handle);
}

// ======================================  Dicts: ======================================  
RECOMP_DLL_FUNC(PythonNative_Dict_Create) {
    py::gil_scoped_acquire gil;

    PyObjectHandle new_handle = controller->create_handle(py::dict());

    RECOMP_RETURN(PyObjectHandle, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Get) {
    py::gil_scoped_acquire gil;
    py::dict d = RECOMP_ARG_PYOBJECT(0);
    py::object key = RECOMP_ARG_PYOBJECT(1);

    py::object obj = d[key];

    PyObjectHandle retVal = controller->create_handle(obj);
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
       PLOGE << e.what();
        RECOMP_RETURN(int, 0);
    }

    PyObjectHandle handle = controller->create_handle(bytecode);
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
       PLOGE << e.what();
        RECOMP_RETURN(int, 0);
    }

    PyObjectHandle handle = controller->create_handle(bytecode);
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
       PLOGE << e.what();
        RECOMP_RETURN(int, 0);
    }

    PyObjectHandle handle = controller->create_handle(bytecode);
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
       PLOGE << e.what();
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
       PLOGE << e.what();
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
        PLOGE << e.what();
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
        PLOGE << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(result);
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
        PLOGE << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(result);
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
        PLOGE << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}

// Python Functions
RECOMP_DLL_FUNC(PythonNative_Call) {
    py::gil_scoped_acquire gil;
    py::function func = RECOMP_ARG_PYOBJECT(0);
    py::tuple args = RECOMP_ARG(PyObjectHandle, 1) ? RECOMP_ARG_PYOBJECT(1) : py::tuple();
    py::dict kwargs = RECOMP_ARG(PyObjectHandle, 2) ? RECOMP_ARG_PYOBJECT(2) : py::dict();

    try {
        func(*args, **kwargs);
    } catch (py::error_already_set &e) {
        PLOGE << e.what();
        RECOMP_RETURN(unsigned int, 0);
    }

    RECOMP_RETURN(unsigned int, 1);
}

RECOMP_DLL_FUNC(PythonNative_Call_Return) {
    py::gil_scoped_acquire gil;
    py::function func = RECOMP_ARG_PYOBJECT(0);
    py::tuple args = RECOMP_ARG(PyObjectHandle, 1) ? RECOMP_ARG_PYOBJECT(1) : py::tuple();
    py::dict kwargs = RECOMP_ARG(PyObjectHandle, 2) ? RECOMP_ARG_PYOBJECT(2) : py::dict();

    py::object result;
    try {
        result = func(*args, **kwargs);
    } catch (py::error_already_set &e) {
        PLOGE << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}


RECOMP_DLL_FUNC(PythonNative_CallAttr) {
    py::gil_scoped_acquire gil;
    py::object obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple args = RECOMP_ARG(PyObjectHandle, 2) ? RECOMP_ARG_PYOBJECT(2) : py::tuple();
    py::dict kwargs = RECOMP_ARG(PyObjectHandle, 3) ? RECOMP_ARG_PYOBJECT(3) : py::dict();

    try {
        obj.attr((char*)name.c_str())(*args, **kwargs);
    } catch (py::error_already_set &e) {
        PLOGE << e.what();
        RECOMP_RETURN(unsigned int, 0);
    }

    RECOMP_RETURN(unsigned int, 1);
}

RECOMP_DLL_FUNC(PythonNative_CallAttr_Return) {
    py::gil_scoped_acquire gil;
    py::object obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple args = RECOMP_ARG(PyObjectHandle, 2) ? RECOMP_ARG_PYOBJECT(2) : py::tuple();
    py::dict kwargs = RECOMP_ARG(PyObjectHandle, 3) ? RECOMP_ARG_PYOBJECT(3) : py::dict();

    py::object result;
    try {
        result = obj.attr((char*)name.c_str())(*args, **kwargs);
    } catch (py::error_already_set &e) {
        PLOGE << e.what();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(result);
    RECOMP_RETURN(PyObjectHandle, handle);
}