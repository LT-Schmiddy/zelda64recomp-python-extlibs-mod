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

// ======================================  API INIT: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Init) {
    uint32_t log_level = RECOMP_ARG(uint32_t, 0);
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(1);
    uint32_t handle_lookup_mode = RECOMP_ARG(uint32_t, 2);

    fs::path mod_dir(mod_dir_text);

    // Set up logging:
    controller = std::make_shared<PyInterpreterController>((plog::Severity)log_level, mod_dir, handle_lookup_mode);
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
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);

    controller->set_handle_suh(handle, true);

    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_IsValidHandle) {
    controller->set_rdram(rdram);
    // Don't need the GIL for this.
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);

    RECOMP_RETURN(uint32_t, (uint32_t) controller->is_valid_handle(handle));
}

RECOMP_DLL_FUNC(PythonNative_Object_GetSUH) {
    controller->set_rdram(rdram);

    py::gil_scoped_acquire gil;
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);

    RECOMP_RETURN(uint32_t, (uint32_t) controller->get_handle_suh(handle));
}


RECOMP_DLL_FUNC(PythonNative_Object_SetSUH) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    REPY_Handle value = RECOMP_ARG(uint32_t, 1);

    controller->set_handle_suh(handle, value);
}


RECOMP_DLL_FUNC(PythonNative_Object_CopyHandle) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* object = RECOMP_ARG_PYOBJECT(0);
    REPY_Handle new_handle = controller->create_handle(object);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, new_handle);
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
    REPY_Handle handle = controller->create_handle(&mod);
    RECOMP_RETURN(REPY_Handle, handle);
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

PYTHON_OBJECT_CREATECAST(Bool, uint32_t, py::bool_);
PYTHON_OBJECT_CREATECAST(U8, uint8_t, py::int_);
PYTHON_OBJECT_CREATECAST(S8, int8_t, py::int_);
PYTHON_OBJECT_CREATECAST(U16, uint16_t, py::int_);
PYTHON_OBJECT_CREATECAST(S16, int16_t, py::int_);
PYTHON_OBJECT_CREATECAST(U32, uint32_t, py::int_);
PYTHON_OBJECT_CREATECAST(S32, int32_t, py::int_);
PYTHON_OBJECT_CREATECAST(F32, float, py::float_);

// ====================================== 64-bit Casting: ======================================

#define NUMBER64_OPERATION(name, c_type, py_type) \
RECOMP_DLL_FUNC(name) { \
    controller->set_rdram(rdram); \
    py::gil_scoped_acquire gil; \
    PTR(c_type) location = RECOMP_ARG(PTR(c_type), 0); \
    /* 0 for read, 1 for write */ \
    uint32_t op_mode = RECOMP_ARG(uint32_t, 1); \
    if (op_mode) { \
        /* Writing: */ \
        /* We only need this if we're writing, and use while reading will give an error: */ \
        py_type* obj = (py_type*)RECOMP_ARG_PYOBJECT(1); \
        c_type val = obj->cast<c_type>(); \
        memcpy_rev_to_recomp(rdram, location, (uint8_t*)&val, sizeof(c_type)); \
        controller->release_suh_handles(); \
        RECOMP_RETURN(REPY_Handle, 0); \
    } else { \
        /* Reading: */ \
        c_type val; \
        memcpy_rev_from_recomp(rdram, (uint8_t*)&val, location, sizeof(c_type)); \
        py_type obj = py_type(val); \
        REPY_Handle retVal = controller->create_handle(&obj); \
        RECOMP_RETURN(REPY_Handle, retVal); \
    } \
}

NUMBER64_OPERATION(PythonNative_Object_U64Operation, uint64_t, py::int_);
NUMBER64_OPERATION(PythonNative_Object_S64Operation, int64_t, py::int_);
NUMBER64_OPERATION(PythonNative_Object_F64Operation, double, py::float_);


// ======================================  String Casting: ======================================
RECOMP_DLL_FUNC(PythonNative_Object_CreateStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);

    py::str obj = py::str(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
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
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Object_CreateByteStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);

    py::bytes obj = py::bytes(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
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

// ====================================== Memcpy: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Memcpy_ToBytes) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(void) data_ptr = RECOMP_ARG(PTR(void), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    uint32_t reverse = RECOMP_ARG(uint32_t, 2);


    uint8_t* mem_block = new uint8_t[data_size];
    if (reverse) {
        memcpy_rev_from_recomp(rdram, mem_block, data_ptr, data_size);
    } else {
        memcpy_from_recomp(rdram, mem_block, data_ptr, data_size);
    }
    py::bytes obj = py::bytes((char*)mem_block, data_size);
    REPY_Handle retVal = controller->create_handle(&obj);
    delete[] mem_block;

    RECOMP_RETURN(REPY_Handle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_Memcpy_FromBytes) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(char) data_ptr = RECOMP_ARG(PTR(char), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    uint32_t reverse = RECOMP_ARG(uint32_t, 2);
    py::bytes* bytes_obj = (py::bytes*)RECOMP_ARG_PYOBJECT(3);

    char* mem_block = new char[data_size];
    uint32_t iter = 0;
    uint32_t bytes_size = py::len(*bytes_obj);
    
    for (auto byte : *bytes_obj) {
        if (iter >= data_size) {
            break;
        }
        mem_block[iter] = byte.cast<uint8_t>();
        iter++;
    }

    if (reverse) {
        memcpy_rev_to_recomp(rdram, data_ptr, mem_block, data_size);
    } else {
        memcpy_to_recomp(rdram, data_ptr, mem_block, data_size);
    }

    delete[] mem_block;
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, iter);
}

RECOMP_DLL_FUNC(PythonNative_Memcpy_ToByteArray) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(void) data_ptr = RECOMP_ARG(PTR(void), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    uint32_t reverse = RECOMP_ARG(uint32_t, 2);


    uint8_t* mem_block = new uint8_t[data_size];
    if (reverse) {
        memcpy_rev_from_recomp(rdram, mem_block, data_ptr, data_size);
    } else {
        memcpy_from_recomp(rdram, mem_block, data_ptr, data_size);
    }
    py::bytearray obj = py::bytearray((char*)mem_block, data_size);
    REPY_Handle retVal = controller->create_handle(&obj);
    delete[] mem_block;

    RECOMP_RETURN(REPY_Handle, retVal);
}


RECOMP_DLL_FUNC(PythonNative_Memcpy_FromByteArray) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    PTR(char) data_ptr = RECOMP_ARG(PTR(char), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    uint32_t reverse = RECOMP_ARG(uint32_t, 2);
    py::bytearray* bytes_obj = (py::bytearray*)RECOMP_ARG_PYOBJECT(3);

    char* mem_block = new char[data_size];
    uint32_t iter = 0;
    uint32_t bytes_size = py::len(*bytes_obj);
    for (auto byte : *bytes_obj) {
        if (iter >= data_size) {
            break;
        }
        mem_block[iter] = byte.cast<uint8_t>();
        iter++;
    }

    if (reverse) {
        memcpy_rev_to_recomp(rdram, data_ptr, mem_block, data_size);
    } else {
        memcpy_to_recomp(rdram, data_ptr, mem_block, data_size);
    }

    delete[] mem_block;
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, iter);
}

// ====================================== Indexing and Slicing Operations: ====================================== 

RECOMP_DLL_FUNC(PythonNative_Object_Len) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* object = RECOMP_ARG_PYOBJECT(0);
    uint32_t len_val = py::len(*object);
    
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, len_val);
}

RECOMP_DLL_FUNC(PythonNative_Object_GetIndex) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::tuple* tuple = (py::tuple*)RECOMP_ARG_PYOBJECT(0); 
    int index = RECOMP_ARG(int, 1); 

    py::object obj = (*tuple)[index];
    REPY_Handle handle = controller->create_handle(&obj);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// ====================================== Iteration: ======================================
RECOMP_DLL_FUNC(PythonNative_Object_Iter) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0); 

    py::object iter = py::iter(*obj);
    REPY_Handle handle = controller->create_handle(&iter);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Object_Next) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0); 
    uint32_t process_stop_iteration = RECOMP_ARG(uint32_t, 2);
    py::object* default_obj; 
    py::object entry;

    try {
        if (RECOMP_ARG(REPY_Handle, 1) != 0) {
            default_obj = RECOMP_ARG_PYOBJECT(1); 
            entry = controller->py_next(*obj, *default_obj);
        } else {
            entry = controller->py_next(*obj);
        }
    } catch (py::error_already_set &e) {
        if (!e.type().is(controller->py_stop_iteration_type) || !process_stop_iteration) {
            controller->handle_exception(&e);
        } else {
            // Catching the exception already clears it from the interpreter. All we need to do now is let it die.
            PLOGD.printf("REPY_Handle %u has ended iteration. StopIteration exception handled internally", RECOMP_ARG(REPY_Handle, 0));
        }
        
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&entry);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// ====================================== Tuple: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Tuple_Create) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t size = RECOMP_ARG(int32_t, 0);
    REPY_Handle* va_args_ptr = RECOMP_ARG(REPY_Handle*, 1);
    
    py::list tmp = py::list();
    for (int i = 0; i < size; i++) {
        tmp.append(controller->get_py_object(va_args_ptr[i]));
    }

    py::tuple new_tuple = py::tuple(tmp);
    REPY_Handle handle = controller->create_handle(&new_tuple);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// ======================================  Dicts: ======================================  
RECOMP_DLL_FUNC(PythonNative_Dict_Create) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict new_dict = py::dict();

    int32_t size = RECOMP_ARG(int32_t, 0);
    REPY_Handle* va_args_ptr = RECOMP_ARG(REPY_Handle*, 1);
    
    for (int i = 0; i < size; i++) {
        py::tuple* pair = (py::tuple*)controller->get_py_object(va_args_ptr[i]);
        new_dict[(*pair)[0]] = (*pair)[1];
    }


    REPY_Handle new_handle = controller->create_handle(&new_dict);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, new_handle);
}

RECOMP_DLL_FUNC(PythonNative_Dict_Get) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    py::object* key = RECOMP_ARG_PYOBJECT(1);

    py::object obj = (*d)[*key];

    REPY_Handle retVal = controller->create_handle(&obj);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, retVal);
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
    
    REPY_Handle retVal = controller->create_handle(&r);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, retVal);
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
        RECOMP_RETURN(REPY_Handle, 0);
    }
    
    REPY_Handle handle = controller->create_handle(&bytecode);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
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
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&bytecode);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
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
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&bytecode);
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Exec) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
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
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
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
    if (RECOMP_ARG(REPY_Handle, 3) == 0) {
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
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
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
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
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
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_EvalCStrN) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict* globals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 3) == 0) {
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
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// Python Functions
RECOMP_DLL_FUNC(PythonNative_Call) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::function* func = (py::function*)RECOMP_ARG_PYOBJECT(0);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    py::tuple* args = RECOMP_ARG(REPY_Handle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : &empty_dict;

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
    py::tuple* args = RECOMP_ARG(REPY_Handle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : &empty_dict;

    py::object result;
    try {
        (*func)(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}


RECOMP_DLL_FUNC(PythonNative_CallAttr) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple empty_tuple = py::tuple();
    py::dict empty_dict = py::dict();
    py::tuple* args = RECOMP_ARG(REPY_Handle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : &empty_tuple;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : &empty_dict;
    
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
    REPY_Handle h1 = RECOMP_ARG(REPY_Handle, 2) ;
    REPY_Handle h2 = RECOMP_ARG(REPY_Handle, 3) ;
    py::tuple* args = h1 ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : &empty_tuple;
    py::dict* kwargs = h2 ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : &empty_dict;
    py::object result;
    try {
        result = obj->attr((char*)name.c_str())(*(*args), **(*kwargs));
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
    
    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// ====================================== Errors: ====================================== 
RECOMP_DLL_FUNC(PythonNative_IsErrorSet) {
    controller->set_rdram(rdram);

    RECOMP_RETURN(int32_t, controller->is_error_set());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorType) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(REPY_Handle, controller->get_py_error_type_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorTrace) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(REPY_Handle, controller->get_py_error_trace_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorValue) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;

    RECOMP_RETURN(REPY_Handle, controller->get_py_error_value_handle());
}

RECOMP_DLL_FUNC(PythonNative_ClearError) {
    controller->set_rdram(rdram);
    py::gil_scoped_acquire gil;
    controller->clear_py_error();
}
