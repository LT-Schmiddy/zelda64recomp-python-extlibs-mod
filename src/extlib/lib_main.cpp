#include <chrono>
#include <random>
#include <unordered_map>
#include <plog/Log.h> // Step1: include the headers

#include "lib_main.hpp"

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

static uint8_t* cached_mem = NULL;

// ======================================  API INIT: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Init) {
    int32_t log_level = RECOMP_ARG(int32_t, 0);
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(1);

    fs::path mod_dir(mod_dir_text);

    // Set up logging:
    controller = std::make_shared<PyInterpreterController>((plog::Severity)log_level, mod_dir);
    controller->set_rdram(rdram);

    PLOGI.printf("Mod Folder: %s", (char*)mod_dir_text.c_str());

    // Release the GIL so that Python threads can run in the background.
    // This does mean that all other functions that operate on python will need to reaquire the GIL.
    RECOMP_RETURN(int, 1);
}
// ======================================  General: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Object_Release) {
    controller->set_rdram(rdram);

    py::gil_scoped_acquire gil;
    int handle = RECOMP_ARG(int, 0);

    controller->release_handle(handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_MakeSUH) {
    controller->set_rdram(rdram);

    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    controller->set_handle_suh(handle, true);

    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_IsValidHandle) {
    controller->set_rdram(rdram);
    // Don't need the GIL for this.
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    RECOMP_RETURN(uint32_t, (uint32_t) controller->is_valid_handle(handle));
}

RECOMP_DLL_FUNC(PythonNative_Object_GetSUH) {
    controller->set_rdram(rdram);

    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);

    RECOMP_RETURN(uint32_t, (uint32_t) controller->get_handle_suh(handle));
}


RECOMP_DLL_FUNC(PythonNative_Object_SetSUH) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PyObjectHandle handle = RECOMP_ARG(PyObjectHandle, 0);
    PyObjectHandle value = RECOMP_ARG(uint32_t, 1);

    controller->set_handle_suh(handle, value);
}


RECOMP_DLL_FUNC(PythonNative_Object_CopyHandle) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* object = RECOMP_ARG_PYOBJECT(0);
    PyObjectHandle new_handle = controller->create_handle(object);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, new_handle);
}

// ======================================  Modules: ====================================== 
RECOMP_DLL_FUNC(PythonNative_LoadModule) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);
    std::string code_string = RECOMP_ARG_STR(1);

    controller->construct_module(module_name, code_string, true);
}

RECOMP_DLL_FUNC(PythonNative_LoadModuleN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);
    int32_t code_len = RECOMP_ARG(int32_t, 2);
    std::string code_string = RECOMP_ARG_STR_N(1, code_len);

    controller->construct_module(module_name, code_string, true);
}

RECOMP_DLL_FUNC(PythonNative_ImportModule) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string module_name = RECOMP_ARG_STR(0);

    py::module_ mod = py::module_::import(module_name.c_str());
    PyObjectHandle handle = controller->create_handle(&mod);
    RECOMP_RETURN(PyObjectHandle, handle);
}

// ====================================== Primative Casting: ======================================  
#define PYTHON_OBJECT_CREATE(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    controller->set_rdram(rdram); \
    py::gil_scoped_acquire gil; \
    c_type value = RECOMP_ARG(c_type, 0); \
    py_type obj = py_type(value); \
    int new_handle = controller->create_handle((py::object*)&obj); \
    RECOMP_RETURN(int, new_handle); \
}

#define PYTHON_OBJECT_CAST(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    controller->set_rdram(rdram); \
    py::gil_scoped_acquire gil; \
    py_type* obj = (py_type*)RECOMP_ARG_PYOBJECT(0); \
    c_type retVal = obj->cast<c_type>(); \
    controller->release_suh_handles(); \
    RECOMP_RETURN(c_type, retVal); \
}

#define PYTHON_OBJECT_CREATECAST(fname, c_type, py_type) \
PYTHON_OBJECT_CREATE(PythonNative_Object_Create ## fname, c_type, py_type); \
PYTHON_OBJECT_CAST(PythonNative_Object_Cast ## fname, c_type, py_type); \

PYTHON_OBJECT_CREATECAST(Bool, int32_t, py::bool_);
PYTHON_OBJECT_CREATECAST(U32, int32_t, py::int_);
PYTHON_OBJECT_CREATECAST(S32, int, py::int_);
PYTHON_OBJECT_CREATECAST(F32, float, py::float_);

// ======================================  Long Long Casting: ======================================

// The arg helper functions don't support anything larger than 32-bit. We'll use address read/write instead.
RECOMP_DLL_FUNC(PythonNative_Object_U64Operation) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(uint64_t) location = RECOMP_ARG(PTR(uint64_t), 0);
    // 0 for read, 1 for write
    uint32_t op_mode = RECOMP_ARG(uint32_t, 1);

    if (op_mode) {
        // Writing:
        // We only need this if we're writing, and use while reading will give an error:
        py::int_* obj = (py::int_*)RECOMP_ARG_PYOBJECT(1);
        uint64_t val = obj->cast<uint64_t>();
        // memcpy_to_recomp(rdram, location, (uint8_t*)"12345678", sizeof(uint64_t));
        memcpy_rev_to_recomp(rdram, location, (uint8_t*)&val, sizeof(uint64_t));
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    } else {
        // Reading:
        uint64_t val;
        memcpy_rev_from_recomp(rdram, (uint8_t*)&val, location, sizeof(uint64_t));
        py::int_ obj = py::int_(val);
        PyObjectHandle retVal = controller->create_handle(&obj);
        RECOMP_RETURN(PyObjectHandle, retVal);
    }
}

RECOMP_DLL_FUNC(PythonNative_Object_S64Operation) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(int64_t) location = RECOMP_ARG(PTR(int64_t), 0);
    // 0 for read, 1 for write
    uint32_t op_mode = RECOMP_ARG(uint32_t, 1);

    if (op_mode) {
        // Writing:
        // We only need this if we're writing, and use while reading will give an error:
        py::int_* obj = (py::int_*)RECOMP_ARG_PYOBJECT(1);
        int64_t val = obj->cast<int64_t>();
        memcpy_rev_to_recomp(rdram, location, (uint8_t*)&val, sizeof(int64_t));
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    } else {
        // Reading:
        int64_t val;
        memcpy_rev_from_recomp(rdram, (uint8_t*)&val, location, sizeof(int64_t));
        py::int_ obj = py::int_(val);
        PyObjectHandle retVal = controller->create_handle(&obj);
        RECOMP_RETURN(PyObjectHandle, retVal);
    }
}


RECOMP_DLL_FUNC(PythonNative_Object_F64Operation) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(double) location = RECOMP_ARG(PTR(double), 0);
    // 0 for read, 1 for write
    uint32_t op_mode = RECOMP_ARG(uint32_t, 1);

    if (op_mode) {
        // Writing:
        // We only need this if we're writing, and use while reading will give an error:
        py::int_* obj = (py::int_*)RECOMP_ARG_PYOBJECT(1);
        double val = obj->cast<double>();
        memcpy_rev_to_recomp(rdram, location, (uint8_t*)&val, sizeof(double));
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    } else {
        // Reading:
        double val;
        memcpy_rev_from_recomp(rdram, (uint8_t*)&val, location, sizeof(double));
        py::float_ obj = py::float_(val);
        PyObjectHandle retVal = controller->create_handle(&obj);
        RECOMP_RETURN(PyObjectHandle, retVal);
    }
}

// ======================================  String Casting: ======================================
RECOMP_DLL_FUNC(PythonNative_Object_CreateStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CastStr_Prepare) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::str* str = (py::str*)RECOMP_ARG_PYOBJECT(0);
    cached_return_u8string = str->cast<std::u8string>();
    controller->release_suh_handles();
    RECOMP_RETURN(int32_t, cached_return_u8string.size());
}

RECOMP_DLL_FUNC(PythonNative_Object_CastStr_Copy) {
    controller->set_rdram(rdram);
    // Don't need the GIL for this step.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_u8string.at(i);
    }
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateByteStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    PyObjectHandle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateByteStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);

    py::bytes obj = py::bytes(value);
    PyObjectHandle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CastByteStr_Prepare) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::str* str = (py::str*)RECOMP_ARG_PYOBJECT(0);
    cached_return_string = str->cast<std::string>();
    controller->release_suh_handles();
    RECOMP_RETURN(int32_t, cached_return_string.size());
}

RECOMP_DLL_FUNC(PythonNative_Object_CastByteStr_Copy) {
    controller->set_rdram(rdram);
    // Don't actually need the GIL for this one.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_string.at(i);
    }
}
// ======================================  Tuple: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Tuple_Create) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t size = RECOMP_ARG(int32_t, 0);
    PyObjectHandle* va_args_ptr = RECOMP_ARG(PyObjectHandle*, 1);
    
    py::list tmp = py::list();
    for (int i = 0; i < size; i++) {
        tmp.append(controller->get_py_object(va_args_ptr[i]));
    }

    py::tuple new_tuple = py::tuple(tmp);
    PyObjectHandle handle = controller->create_handle(&new_tuple);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Tuple_GetMember) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::tuple* tuple = (py::tuple*)RECOMP_ARG_PYOBJECT(0); 
    int index = RECOMP_ARG(int, 1); 

    py::object obj = (*tuple)[index];
    PyObjectHandle handle = controller->create_handle(&obj);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

// ======================================  Dicts: ======================================  
RECOMP_DLL_FUNC(PythonNative_Dict_Create) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict new_dict = py::dict();

    int32_t size = RECOMP_ARG(int32_t, 0);
    PyObjectHandle* va_args_ptr = RECOMP_ARG(PyObjectHandle*, 1);
    
    for (int i = 0; i < size; i++) {
        py::tuple* pair = (py::tuple*)controller->get_py_object(va_args_ptr[i]);
        new_dict[(*pair)[0]] = (*pair)[1];
    }


    PyObjectHandle new_handle = controller->create_handle(&new_dict);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Get) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    py::object obj = (*d)[*key];

    PyObjectHandle retVal = controller->create_handle(&obj);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Set) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);
    py::object* value = RECOMP_ARG_PYOBJECT(2);

    (*d)[*key] = value;
    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_Dict_Has) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    uint32_t retVal = d->contains(key);
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Remove) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    d->attr("pop")(key);
    controller->release_suh_handles();
}

// ====================================== Object Attributes: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Object_GetAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);
    
    py::object r;
    // If a default was passed:
    if (RECOMP_ARG(uint32_t, 2)) {
        py::object* default_r = RECOMP_ARG_PYOBJECT(2);
        r = py::getattr(*obj, *key, *default_r);
    } else {
        r = py::getattr(*obj, *key);
    }
    
    PyObjectHandle retVal = controller->create_handle(&r);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_SetAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);
    py::object* value = RECOMP_ARG_PYOBJECT(2);

    py::setattr(*obj, *key, *value);
    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_Object_HasAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    uint32_t retVal = py::hasattr(*obj, *key);
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_DelAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    py::delattr(*obj, *key);
    
    controller->release_suh_handles();
}

// ====================================== Execution: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Compile) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::str* code_str = (py::str*)RECOMP_ARG_PYOBJECT(0);
    py::str* identifier_str = (py::str*)RECOMP_ARG_PYOBJECT(1);
    py::str* type_str = (py::str*)RECOMP_ARG_PYOBJECT(2);

    py::object bytecode;
    try {
        bytecode = controller->py_compile(code_str, identifier_str, type_str);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }
    
    PyObjectHandle handle = controller->create_handle(&bytecode);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CompileCStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string code_str = RECOMP_ARG_STR(0);
    std::string identifier = RECOMP_ARG_STR(1);
    int32_t code_type = RECOMP_ARG(int32_t, 2);

    py::object bytecode;
    try {
        bytecode = controller->py_compile(code_str, identifier, code_type_strs[code_type]);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&bytecode);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CompileCStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_str = RECOMP_ARG_STR_N(0, code_len);
    std::string identifier = RECOMP_ARG_STR(2);
    int32_t code_type = RECOMP_ARG(int32_t, 3);

    py::object bytecode;
    try {
        bytecode = controller->py_compile(code_str, identifier, code_type_strs[code_type]);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&bytecode);
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Exec) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    try {
        controller->py_exec(bytecode, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(uint32_t, 0);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    try {
        controller->py_exec(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(uint32_t, 0);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, 1);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 3) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(3);
    }

    try {
        controller->py_exec(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(uint32_t, 0);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, 1);
}

RECOMP_DLL_FUNC(PythonNative_Eval) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }
    
    py::object result;
    try {
        result = controller->py_eval(bytecode, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    py::object result;
    try {
        result = controller->py_eval(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    py::dict* locals;
    if (RECOMP_ARG(PyObjectHandle, 3) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(3);
    }

    py::object result;
    try {
        result = controller->py_eval(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

// Python Functions
RECOMP_DLL_FUNC(PythonNative_Call) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::function* func = (py::function*)RECOMP_ARG_PYOBJECT(0);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    py::tuple* args = RECOMP_ARG(PyObjectHandle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(PyObjectHandle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : &empty_dict;

    try {
        (*func)(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(uint32_t, 0);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, 1);
}

RECOMP_DLL_FUNC(PythonNative_Call_Return) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::function* func = (py::function*)RECOMP_ARG_PYOBJECT(0);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    py::tuple* args = RECOMP_ARG(PyObjectHandle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(PyObjectHandle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : &empty_dict;

    py::object result;
    try {
        (*func)(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }

    PyObjectHandle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}


RECOMP_DLL_FUNC(PythonNative_CallAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    py::tuple* args = RECOMP_ARG(PyObjectHandle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(PyObjectHandle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : &empty_dict;
    
    try {
        obj->attr((char*)name.c_str())(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(uint32_t, 0);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, 1);
}

RECOMP_DLL_FUNC(PythonNative_CallAttr_Return) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    PyObjectHandle h1 = RECOMP_ARG(PyObjectHandle, 2) ;
    PyObjectHandle h2 = RECOMP_ARG(PyObjectHandle, 3) ;
    py::tuple* args = h1 ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : &empty_tuple;
    py::dict* kwargs = h2 ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : &empty_dict;
    py::object result;
    try {
        result = obj->attr((char*)name.c_str())(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(PyObjectHandle, 0);
    }
    
    PyObjectHandle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(PyObjectHandle, handle);
}

// ====================================== Errors: ====================================== 
RECOMP_DLL_FUNC(PythonNative_IsErrorSet) {
    controller->set_rdram(rdram);

    RECOMP_RETURN(int32_t, controller->is_error_set());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorType) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(PyObjectHandle, controller->get_py_error_type_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorTrace) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(PyObjectHandle, controller->get_py_error_trace_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorValue) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(PyObjectHandle, controller->get_py_error_value_handle());
}

RECOMP_DLL_FUNC(PythonNative_ClearError) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    controller->clear_py_error();
}

