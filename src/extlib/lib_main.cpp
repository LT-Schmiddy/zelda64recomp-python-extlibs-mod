#include <chrono>
#include <random>
#include <queue>
#include <format>
#include <algorithm>
#include <plog/Log.h> // Step 1: include the headers

#include "lib_main.hpp"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

// This used to be bigger, but I'm keeping it.
#define INTERP_API_HEADER \
    controller->thread_check_warning(); \
    controller->set_rdram(rdram); \


static const char* code_type_strs[] = {
    "exec",
    "eval",
    "single"
};
static std::queue<fs::path> preinit_module_nrms;
// Test
RECOMP_DLL_FUNC(PythonNative_Test) {
	std::cout << "Hello from REPY\n";
	std::cout << RECOMP_ARG(uint32_t, 0) << "\n";
}

// 
RECOMP_DLL_FUNC(PythonNative_Preinit_RegisterNrmInModuleSearchPath) {
    ZoneScoped;
    std::u8string nrm_path_str = RECOMP_ARG_U8STR(0);
    fs::path nrm_path(nrm_path_str);

    preinit_module_nrms.push(nrm_path);
}

// ======================================  API INIT: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Init) {
    ZoneScoped;
    std::cout << "Hello Alex\n";
    uint32_t log_level = RECOMP_ARG(uint32_t, 0);
    uint32_t log_to_file = RECOMP_ARG(uint32_t, 1);
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(2);
    fs::path mod_dir(mod_dir_text);
    
    // Set up logging:
    controller = std::make_unique<PyInterpreterController>((plog::Severity)log_level, log_to_file, mod_dir, &preinit_module_nrms);
    controller->set_rdram(rdram);

    PLOGI.printf("Mod Folder: %s", (char*)mod_dir_text.c_str());
    RECOMP_RETURN(int, 1);
}
// ======================================  General: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Release) {
    ZoneScoped;
    INTERP_API_HEADER;

    int handle = RECOMP_ARG(int, 0);
    controller->release_handle(handle);
}

RECOMP_DLL_FUNC(PythonNative_MakeSUH) {
    ZoneScoped;
    INTERP_API_HEADER;
    // Don't need the API header for this
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    controller->set_handle_suh(handle, true);

    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_IsValidHandle) {
    ZoneScoped;
    INTERP_API_HEADER;
    // Don't need the API header for this
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    RECOMP_RETURN(bool, controller->is_valid_handle(handle));
}

RECOMP_DLL_FUNC(PythonNative_GetSUH) {
    ZoneScoped;
    INTERP_API_HEADER;
    // Don't need the API header for this
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    RECOMP_RETURN(bool, controller->get_handle_suh(handle));
}

RECOMP_DLL_FUNC(PythonNative_SetSUH) {
    ZoneScoped;
    INTERP_API_HEADER;
    // Don't need the API header for this
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    REPY_Handle value = RECOMP_ARG(bool, 1);
    controller->set_handle_suh(handle, value);
}

RECOMP_DLL_FUNC(PythonNative_CopyHandle) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* object = RECOMP_ARG_PYOBJECT(0);
    REPY_Handle new_handle = controller->create_handle(object);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, new_handle);
}

// ======================================  Subcontrollers/Subinterpreters: ====================================== 
RECOMP_DLL_FUNC(PythonNative_RegisterSubinterpreter) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::gil_scoped_acquire gil; // Still needed, since this can be used with an empty interpreter stack, and the GIL is needed for this.
    REPY_InterpreterIndex retVal = controller->create_subcontroller();
    RECOMP_RETURN(REPY_InterpreterIndex, retVal);
}

RECOMP_DLL_FUNC(PythonNative_PushInterpreter) {
    ZoneScoped;
    INTERP_API_HEADER;
    REPY_InterpreterIndex interp = RECOMP_ARG(REPY_InterpreterIndex, 0);
    controller->push_subcontroller_index(interp);
}

RECOMP_DLL_FUNC(PythonNative_PopInterpreter) {
    ZoneScoped;
    INTERP_API_HEADER;
    controller->pop_subcontroller_index();
}

RECOMP_DLL_FUNC(PythonNative_GetCurrentInterpreter) {
    ZoneScoped;
    INTERP_API_HEADER;
    REPY_InterpreterIndex retVal = controller->get_current_subcontroller_index();
    RECOMP_RETURN(REPY_InterpreterIndex, retVal);
}

RECOMP_DLL_FUNC(PythonNative_GetInterpreterAutoDisarm) {
    ZoneScoped;
    INTERP_API_HEADER;
    REPY_InterpreterIndex index = RECOMP_ARG(REPY_InterpreterIndex, 0);
    PySubController* sc = controller->get_subcontroller(index);
    u32 retVal = sc->get_auto_disarm();
    RECOMP_RETURN(u32, retVal);
}

RECOMP_DLL_FUNC(PythonNative_SetInterpreterAutoDisarm) {
    ZoneScoped;
    INTERP_API_HEADER;
    REPY_InterpreterIndex index = RECOMP_ARG(REPY_InterpreterIndex, 0);
    u32 val = RECOMP_ARG(u32, 1);
    PySubController* sc = controller->get_subcontroller(index);
    sc->set_auto_disarm(val);
}

RECOMP_DLL_FUNC(PythonNative_GetHandleInterpreter) {
    ZoneScoped;
    INTERP_API_HEADER;
    REPY_Handle handle = RECOMP_ARG(REPY_Handle, 0);
    REPY_InterpreterIndex retVal = controller->get_py_object_interpreter(handle);
    RECOMP_RETURN(REPY_InterpreterIndex, retVal);
}

// ======================================  Modules: ====================================== 
RECOMP_DLL_FUNC(PythonNative_AddCStrToSysPath) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string new_path = RECOMP_ARG_U8STR(0);
    auto sys = py::module_::import("sys");
    py::list sys_path = sys.attr("path");
    sys_path.append((const char*) new_path.c_str());
    PLOGI.printf("Adding '%s' to sys.path", new_path.c_str());
}


RECOMP_DLL_FUNC(PythonNative_ConstructModuleFromCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string module_name = RECOMP_ARG_U8STR(0);
    std::u8string code_string = RECOMP_ARG_U8STR(1);
    bool add_to_sys = RECOMP_ARG(bool, 2);

    controller->construct_module(module_name, code_string, add_to_sys);
}

RECOMP_DLL_FUNC(PythonNative_ConstructModuleFromCStrN) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string module_name = RECOMP_ARG_U8STR(0);
    int32_t code_len = RECOMP_ARG(int32_t, 2);
    std::u8string code_string = RECOMP_ARG_U8STR_N(1, code_len);
    bool add_to_sys = RECOMP_ARG(bool, 3);

    controller->construct_module(module_name, code_string, add_to_sys);
}

RECOMP_DLL_FUNC(PythonNative_ImportModule) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string module_name = RECOMP_ARG_U8STR(0);

    try {
        py::module_ mod = py::module_::import((const char *)module_name.c_str());
        REPY_Handle handle = controller->create_handle(&mod);
        RECOMP_RETURN(REPY_Handle, handle);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

// ====================================== Primative Casting: ======================================  
#define PYTHON_OBJECT_CREATE(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    ZoneScoped; \
    INTERP_API_HEADER \
    c_type value = RECOMP_ARG(c_type, 0); \
    py_type obj = py_type(value); \
    int new_handle = controller->create_handle((py::object*)&obj); \
    RECOMP_RETURN(int, new_handle); \
}

#define PYTHON_OBJECT_CAST(fname, c_type, py_type) \
RECOMP_DLL_FUNC(fname) { \
    ZoneScoped; \
    INTERP_API_HEADER \
    py_type* obj = (py_type*)RECOMP_ARG_PYOBJECT(0); \
    c_type retVal = obj->cast<c_type>(); \
    controller->release_suh_handles(); \
    RECOMP_RETURN(c_type, retVal); \
}

#define PYTHON_OBJECT_CREATECAST(fname, c_type, py_type) \
PYTHON_OBJECT_CREATE(PythonNative_Create ## fname, c_type, py_type); \
PYTHON_OBJECT_CAST(PythonNative_Cast ## fname, c_type, py_type); \

PYTHON_OBJECT_CREATECAST(Bool, bool, py::bool_);
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
    ZoneScoped; \
    INTERP_API_HEADER \
    PTR(c_type) location = RECOMP_ARG(PTR(c_type), 0); \
    /* 0 for read, 1 for write */ \
    bool op_mode = RECOMP_ARG(bool, 1); \
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

NUMBER64_OPERATION(PythonNative_U64Operation, uint64_t, py::int_);
NUMBER64_OPERATION(PythonNative_S64Operation, int64_t, py::int_);
NUMBER64_OPERATION(PythonNative_F64Operation, double, py::float_);


// ======================================  String Casting: ======================================
RECOMP_DLL_FUNC(PythonNative_CreateStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string value = RECOMP_ARG_U8STR(0);

    py::str obj = py::str(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_CreateStrN) {
    ZoneScoped;
    INTERP_API_HEADER;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::u8string value = RECOMP_ARG_U8STR_N(0, str_len);

    py::str obj = py::str(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

// Cast and allocate str. Both functions are always called in succession
// by mod-code, which allows this to work correctly.
thread_local static std::u8string cached_return_u8string;
RECOMP_DLL_FUNC(PythonNative_CastStr_Prepare) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::str* str = (py::str*)RECOMP_ARG_PYOBJECT(0);
    cached_return_u8string = str->cast<std::u8string>();
    controller->release_suh_handles();
    RECOMP_RETURN(int32_t, cached_return_u8string.size());
}

RECOMP_DLL_FUNC(PythonNative_CastStr_Copy) {
    ZoneScoped;
    controller->set_rdram(rdram);
    // Don't need the GIL for this step.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_u8string.at(i);
    }
}

RECOMP_DLL_FUNC(PythonNative_CreateByteStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::string value = RECOMP_ARG_STR(0);

    py::bytes obj = py::bytes(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

RECOMP_DLL_FUNC(PythonNative_CreateByteStrN) {
    ZoneScoped;
    INTERP_API_HEADER;
    int32_t str_len = RECOMP_ARG(int32_t, 1);
    std::string value = RECOMP_ARG_STR_N(0, str_len);

    py::bytes obj = py::bytes(value);
    REPY_Handle retVal = controller->create_handle(&obj);
    RECOMP_RETURN(REPY_Handle, retVal);
}

// Cast and allocate bytes. Both functions are always called in succession
// by mod-code, which allows this to work correctly.
thread_local static std::string cached_return_string;
RECOMP_DLL_FUNC(PythonNative_CastByteStr_Prepare) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::bytes* str = (py::bytes*)RECOMP_ARG_PYOBJECT(0);
    cached_return_string = str->cast<std::string>();
    controller->release_suh_handles();
    RECOMP_RETURN(int32_t, cached_return_string.size());
}

RECOMP_DLL_FUNC(PythonNative_CastByteStr_Copy) {
    ZoneScoped;
    controller->set_rdram(rdram);
    // Don't actually need the GIL for this one.
    int str_len = RECOMP_ARG(int, 0);
    PTR(char) str_ptr = RECOMP_ARG(PTR(char), 1);

    for (int i = 0; i < str_len; i++) {
        MEM_B(str_ptr, i) = cached_return_string.at(i);
    }
}

// ====================================== Memcpy: ====================================== 
RECOMP_DLL_FUNC(PythonNative_MemcpyToBytes) {
    ZoneScoped;
    INTERP_API_HEADER;
    PTR(void) data_ptr = RECOMP_ARG(PTR(void), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    bool reverse = RECOMP_ARG(bool, 2);


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

RECOMP_DLL_FUNC(PythonNative_MemcpyFromBytes) {
    ZoneScoped;
    INTERP_API_HEADER;
    PTR(char) data_ptr = RECOMP_ARG(PTR(char), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    bool reverse = RECOMP_ARG(bool, 2);
    py::bytes* bytes_obj = (py::bytes*)RECOMP_ARG_PYOBJECT(3);

    py::buffer_info info = py::buffer(*bytes_obj).request();
    uint8_t* buf_data = (uint8_t*)info.ptr;
    py::ssize_t buf_size = std::min(info.size, (py::ssize_t)data_size); 
    if (reverse) {
        memcpy_rev_to_recomp(rdram, data_ptr, buf_data, buf_size);
    } else {
        memcpy_to_recomp(rdram, data_ptr, buf_data, buf_size);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, buf_size);
}

RECOMP_DLL_FUNC(PythonNative_MemcpyToByteArray) {
    ZoneScoped;
    INTERP_API_HEADER;
    PTR(void) data_ptr = RECOMP_ARG(PTR(void), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    bool reverse = RECOMP_ARG(bool, 2);


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


RECOMP_DLL_FUNC(PythonNative_MemcpyFromByteArray) {
    ZoneScoped;
    INTERP_API_HEADER;
    PTR(char) data_ptr = RECOMP_ARG(PTR(char), 0);
    uint32_t data_size = RECOMP_ARG(uint32_t, 1);
    bool reverse = RECOMP_ARG(bool, 2);
    py::bytearray* bytes_obj = (py::bytearray*)RECOMP_ARG_PYOBJECT(3);

    py::buffer_info info = py::buffer(*bytes_obj).request();
    uint8_t* buf_data = (uint8_t*)info.ptr;
    py::ssize_t buf_size = std::min(info.size, (py::ssize_t)data_size); 
    if (reverse) {
        memcpy_rev_to_recomp(rdram, data_ptr, buf_data, buf_size);
    } else {
        memcpy_to_recomp(rdram, data_ptr, buf_data, buf_size);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, buf_size);
}

// ====================================== Indexing and Slicing Operations: ====================================== 

RECOMP_DLL_FUNC(PythonNative_Len) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* object = RECOMP_ARG_PYOBJECT(0);
    uint32_t len_val = py::len(*object);
    
    controller->release_suh_handles();
    RECOMP_RETURN(uint32_t, len_val);
}

// ====================================== Iteration: ======================================
RECOMP_DLL_FUNC(PythonNative_Iter) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0); 

    py::object iter = py::iter(*obj);
    REPY_Handle handle = controller->create_handle(&iter);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Next) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0); 
    bool process_stop_iteration = RECOMP_ARG(bool, 2);
    py::object* default_obj; 
    py::object entry;

    try {
        if (RECOMP_ARG(REPY_Handle, 1) != 0) {
            default_obj = RECOMP_ARG_PYOBJECT(1); 
            entry = controller->py_next()(*obj, *default_obj);
        } else {
            entry = controller->py_next()(*obj);
        }
    } catch (py::error_already_set &e) {
        if (!e.type().is(controller->py_stop_iteration_type()) || !process_stop_iteration) {
            controller->handle_exception(&e);
        } else {
            // Catching the exception already clears it from the interpreter. All we need to do now is let it die.
            PLOGD.printf("REPY_Handle 0x%08X has ended iteration. StopIteration exception handled internally", RECOMP_ARG(REPY_Handle, 0));
        }
        
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&entry);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

// ====================================== Tuple: ====================================== 
RECOMP_DLL_FUNC(PythonNative_CreateTuple) {
    ZoneScoped;
    INTERP_API_HEADER;
    uint32_t size = RECOMP_ARG(uint32_t, 0);
    REPY_Handle* va_args_ptr = RECOMP_ARG(REPY_Handle*, 1);
    
    try {
        py::list tmp = py::list();
        for (int i = 0; i < size; i++) {
            tmp.append(controller->get_py_object(va_args_ptr[i]));
            FrameMark;
        }

        py::tuple new_tuple = py::tuple(tmp);
        REPY_Handle handle = controller->create_handle(&new_tuple);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, handle);
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

}

RECOMP_DLL_FUNC(PythonNative_TupleGetIndexS32) {
    ZoneScoped;
    INTERP_API_HEADER;

    py::tuple* tuple = (py::tuple*)RECOMP_ARG_PYOBJECT(0); 
    int index = RECOMP_ARG(int, 1); 

    try {
        py::object obj = (*tuple)[index];
        
        REPY_Handle handle = controller->create_handle(&obj);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, handle);
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_CreatePairCStr) {
    ZoneScoped;
    INTERP_API_HEADER;

    std::u8string key = RECOMP_ARG_U8STR(0); 
    py::object* value = RECOMP_ARG_PYOBJECT(1); 

    try {
        py::list tmp = py::list();
        tmp.append(py::str(key));
        tmp.append(*value);
        py::tuple new_tuple = py::tuple(tmp);
        REPY_Handle handle = controller->create_handle(&new_tuple);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, handle);
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}


// ======================================  Dicts: ======================================  
RECOMP_DLL_FUNC(PythonNative_CreateDict) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::dict new_dict = py::dict();

    uint32_t size = RECOMP_ARG(uint32_t, 0);
    REPY_Handle* va_args_ptr = RECOMP_ARG(REPY_Handle*, 1);
    try {
        for (int i = 0; i < size; i++) {
            py::tuple* pair = (py::tuple*)controller->get_py_object(va_args_ptr[i]);
            new_dict[(*pair)[0]] = (*pair)[1];
            FrameMark;
        }

        REPY_Handle new_handle = controller->create_handle(&new_dict);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, new_handle);

    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_DictGet) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);

        py::object obj = (*d)[*key];

        REPY_Handle retVal = controller->create_handle(&obj);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, retVal);

    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_DictGetCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);

        py::object obj = (*d)[(char*)key.c_str()];

        REPY_Handle retVal = controller->create_handle(&obj);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, retVal);

    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_DictSet) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);
        py::object* value = RECOMP_ARG_PYOBJECT(2);

        (*d)[*key] = value;
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_DictSetCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);
        py::object* value = RECOMP_ARG_PYOBJECT(2);

        (*d)[(char*)key.c_str()] = value;
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_DictHas) {
    ZoneScoped;
    INTERP_API_HEADER;
    bool retVal = false;

    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);

        retVal = d->contains(key);
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(bool, retVal);
}

RECOMP_DLL_FUNC(PythonNative_DictHasCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    bool retVal = false;

    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);

        retVal = d->contains(key.c_str());
    
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(bool, retVal);
}

RECOMP_DLL_FUNC(PythonNative_DictDel) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);

        d->attr("pop")(*key);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_DictDelCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::dict* d = (py::dict*)RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);

        d->attr("pop")(key.c_str());

    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
}

// ====================================== Object Attributes: ====================================== 
RECOMP_DLL_FUNC(PythonNative_GetAttr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);
        
        py::object r;
        // If a default was passed:
        if (RECOMP_ARG(REPY_Handle, 2)) {
            py::object* default_r = RECOMP_ARG_PYOBJECT(2);
            r = py::getattr(*obj, *key, *default_r);
        } else {
            r = py::getattr(*obj, *key);
        }
        
        REPY_Handle retVal = controller->create_handle(&r);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, retVal);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_GetAttrCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);
        
        py::object r;
        // If a default was passed:
        if (RECOMP_ARG(REPY_Handle, 2)) {
            py::object* default_r = RECOMP_ARG_PYOBJECT(2);
            r = py::getattr(*obj, (char*)key.c_str(), *default_r);
        } else {
            r = py::getattr(*obj, (char*)key.c_str());
        }
        
        REPY_Handle retVal = controller->create_handle(&r);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, retVal);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
}

RECOMP_DLL_FUNC(PythonNative_SetAttr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);
        py::object* value = RECOMP_ARG_PYOBJECT(2);

        py::setattr(*obj, *key, *value);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_SetAttrCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);
        py::object* value = RECOMP_ARG_PYOBJECT(2);

        py::setattr(*obj, (char*)key.c_str(), *value);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
    }

    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_HasAttr) {
    ZoneScoped;
    INTERP_API_HEADER;
    bool retVal = 0;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);

        retVal = py::hasattr(*obj, *key);
    } catch(py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(bool, retVal);
}

RECOMP_DLL_FUNC(PythonNative_HasAttrCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    bool retVal = 0;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);

        retVal = py::hasattr(*obj, (char*)key.c_str());
    } catch(py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(bool, retVal);
}

RECOMP_DLL_FUNC(PythonNative_DelAttr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        py::object* key = RECOMP_ARG_PYOBJECT(1);

        py::delattr(*obj, *key);
    } catch(py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
}

RECOMP_DLL_FUNC(PythonNative_DelAttrCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    try {
        py::object* obj = RECOMP_ARG_PYOBJECT(0);
        std::u8string key = RECOMP_ARG_U8STR(1);

        py::delattr(*obj, (char*)key.c_str());
    } catch(py::error_already_set &e) {
        controller->handle_exception(&e);
    }
    controller->release_suh_handles();
}

// ====================================== Execution: ====================================== 
RECOMP_DLL_FUNC(PythonNative_Compile) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::str* code_str = (py::str*)RECOMP_ARG_PYOBJECT(0);
    py::str* identifier_str = (py::str*)RECOMP_ARG_PYOBJECT(1);
    py::str* type_str = (py::str*)RECOMP_ARG_PYOBJECT(2);

    py::object bytecode;
    try {
        // PLOGD.printf("Compiling %s", identifier_str->cast<std::u8string>().c_str()); // I want to print whenever inline code caches get printed.
        bytecode = controller->py_compile()(code_str, identifier_str, type_str);
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
    ZoneScoped;
    INTERP_API_HEADER;
    std::string code_str = RECOMP_ARG_STR(0);
    std::string identifier = RECOMP_ARG_STR(1);
    int32_t code_type = RECOMP_ARG(int32_t, 2);

    py::object bytecode;
    try {
        // PLOGD.printf("Compiling %s", identifier.c_str()); // I want to print whenever inline code caches get printed.
        bytecode = controller->py_compile()(code_str, identifier, code_type_strs[code_type]);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&bytecode);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CompileCStrN) {
    ZoneScoped;
    INTERP_API_HEADER;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_str = RECOMP_ARG_STR_N(0, code_len);
    std::string identifier = RECOMP_ARG_STR(2);
    int32_t code_type = RECOMP_ARG(int32_t, 3);

    py::object bytecode;
    try {
        // PLOGD.printf("Compiling %s", identifier.c_str()); // I want to print whenever inline code caches get printed.
        bytecode = controller->py_compile()(code_str, identifier, code_type_strs[code_type]);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&bytecode);
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_Exec) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 1) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    }

    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    try {
        controller->py_exec()(bytecode, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 1) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    }
    

    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    try {
        controller->py_exec()(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_ExecCStrN) {
    ZoneScoped;
    INTERP_API_HEADER;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);
    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 2) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 3) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(3);
    }

    try {
        controller->py_exec()(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }
    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_Eval) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* bytecode = RECOMP_ARG_PYOBJECT(0);
    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 1) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    }
    
    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }
    
    py::object result;
    try {
        result = controller->py_eval()(bytecode, globals, locals);
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
    ZoneScoped;
    INTERP_API_HEADER;
    std::string code_string = RECOMP_ARG_STR(0);
    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 1) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(1);
    }
    
    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 2) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    py::object result;
    try {
        result = controller->py_eval()(code_string, globals, locals);
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
    ZoneScoped;
    INTERP_API_HEADER;
    int32_t code_len = RECOMP_ARG(int32_t, 1);
    std::string code_string = RECOMP_ARG_STR_N(0, code_len);

    py::dict empty_globals = py::dict();
    py::dict* globals = &empty_globals; 
    if (RECOMP_ARG(REPY_Handle, 2) != 0) {
        globals = (py::dict*)RECOMP_ARG_PYOBJECT(2);
    }

    py::dict* locals;
    if (RECOMP_ARG(REPY_Handle, 3) == 0) {
        locals = globals;
    } else {
        locals = (py::dict*)RECOMP_ARG_PYOBJECT(3);
    }

    py::object result;
    try {
        result = controller->py_eval()(code_string, globals, locals);
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_VL)  {
    py::dict new_dict;
    py::dict* target_dict = &new_dict;

    // use the provided dict, if one exists.
    REPY_Handle retVal = RECOMP_ARG(REPY_Handle, 0);
    if (retVal) {
        target_dict = (py::dict*)RECOMP_ARG_PYOBJECT(0);
    }

    uint32_t size = RECOMP_ARG(uint32_t, 1);
    REPY_Handle* va_args_ptr = RECOMP_ARG(REPY_Handle*, 2);
    try {
        for (int i = 0; i < size; i++) {
            py::object* obj = controller->get_py_object(va_args_ptr[i]);
            (*target_dict)[py::str(std::format("_{}", i))] = *obj;
            FrameMark;
        }

        // if a dict was provided, don't make a new handle. return the old one.
        if (target_dict == &new_dict) {
            retVal = controller->create_handle(target_dict);
        }
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, retVal);

    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }

}

// Python Functions
RECOMP_DLL_FUNC(PythonNative_Call) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::function* func = (py::function*)RECOMP_ARG_PYOBJECT(0);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : NULL;

    try {
        if (args != NULL && kwargs != NULL) {
            (*func)(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            (*func)(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            (*func)(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            (*func)();
        } 
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_CallReturn) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::function* func = (py::function*)RECOMP_ARG_PYOBJECT(0);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 1) ? (py::tuple*)RECOMP_ARG_PYOBJECT(1) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 2) ? (py::dict*)RECOMP_ARG_PYOBJECT(2) : NULL;

    py::object result;
    try {
        if (args != NULL && kwargs != NULL) {
            result = (*func)(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            result = (*func)(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            result = (*func)(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            result = (*func)();
        } 
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
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* name = RECOMP_ARG_PYOBJECT(1);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : NULL;
    
    try {
        if (args != NULL && kwargs != NULL) {
            obj->attr(*name)(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            obj->attr(*name)(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            obj->attr(*name)(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            obj->attr(*name)();
        } 
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_CallAttrCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : NULL;
    
    try {
        if (args != NULL && kwargs != NULL) {
            obj->attr((char*)name.c_str())(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            obj->attr((char*)name.c_str())(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            obj->attr((char*)name.c_str())(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            obj->attr((char*)name.c_str())();
        } 
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(bool, false);
    }

    controller->release_suh_handles();
    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PythonNative_CallAttrReturn) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    py::object* name = RECOMP_ARG_PYOBJECT(1);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : NULL;
    py::object result;
    try {
        if (args != NULL && kwargs != NULL) {
            result = obj->attr(*name)(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            result = obj->attr(*name)(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            result = obj->attr(*name)(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            result = obj->attr(*name)();
        } 
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
    
    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}

RECOMP_DLL_FUNC(PythonNative_CallAttrCStrReturn) {
    ZoneScoped;
    INTERP_API_HEADER;
    py::object* obj = RECOMP_ARG_PYOBJECT(0);
    std::u8string name = RECOMP_ARG_U8STR(1);
    py::tuple* args = RECOMP_ARG(REPY_Handle, 2) ? (py::tuple*)RECOMP_ARG_PYOBJECT(2) : NULL;
    py::dict* kwargs = RECOMP_ARG(REPY_Handle, 3) ? (py::dict*)RECOMP_ARG_PYOBJECT(3) : NULL;
    py::object result;
    try {
        if (args != NULL && kwargs != NULL) {
            result = obj->attr((char*)name.c_str())(*(*args), **(*kwargs));
        } else if (args != NULL && kwargs == NULL) {
            result = obj->attr((char*)name.c_str())(*(*args));
        } else if (args == NULL && kwargs != NULL) {
            result = obj->attr((char*)name.c_str())(**(*kwargs));
        } else /* if (args == NULL && kwargs == NULL) */ {
            result = obj->attr((char*)name.c_str())();
        } 
    } catch (py::error_already_set &e) {
        controller->handle_exception(&e);
        controller->release_suh_handles();
        RECOMP_RETURN(REPY_Handle, 0);
    }
    
    REPY_Handle handle = controller->create_handle(&result);
    controller->release_suh_handles();
    RECOMP_RETURN(REPY_Handle, handle);
}


// ====================================== Zipfile: ====================================== 
// Quickly generates ZipFile
RECOMP_DLL_FUNC(PythonNative_GetFileFromPathCStr) {
    ZoneScoped;
    INTERP_API_HEADER;
    std::u8string filepath = RECOMP_ARG_U8STR(0);
    REPY_Handle retVal = controller->get_zipfile_from_path(filepath);

    RECOMP_RETURN(REPY_Handle, retVal);
}

// ====================================== Errors: ====================================== 
RECOMP_DLL_FUNC(PythonNative_IsErrorSet) {
    ZoneScoped;
    controller->set_rdram(rdram);

    RECOMP_RETURN(int32_t, controller->is_error_set());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorType) {
    ZoneScoped;
    INTERP_API_HEADER;
    RECOMP_RETURN(REPY_Handle, controller->get_py_error_type_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorTrace) {
    ZoneScoped;
    INTERP_API_HEADER;
    RECOMP_RETURN(REPY_Handle, controller->get_py_error_trace_handle());
}

RECOMP_DLL_FUNC(PythonNative_GetErrorValue) {
    ZoneScoped;
    INTERP_API_HEADER;
    RECOMP_RETURN(REPY_Handle, controller->get_py_error_value_handle());
}

RECOMP_DLL_FUNC(PythonNative_ClearError) {
    ZoneScoped;
    INTERP_API_HEADER;
    controller->clear_py_error();
}

// ====================================== Logging: ====================================== 
// The following enable the mod-code of this library to use PLOG for logging.
thread_local static plog::Severity py_log_severity;
thread_local static std::string py_log_func;
thread_local static uint32_t py_log_line_number;
thread_local static std::string py_log_file_name;

RECOMP_DLL_FUNC(PythonNative_SetLogMetaData) {
    ZoneScoped;
    py_log_severity = (plog::Severity)RECOMP_ARG(uint32_t, 0);
    py_log_func = RECOMP_ARG_STR(1);
    py_log_line_number = RECOMP_ARG(uint32_t, 2);
    py_log_file_name = RECOMP_ARG_STR(3);
}

RECOMP_DLL_FUNC(PythonNative_CommitLogMessage) {
    ZoneScoped;
    std::string log_message = RECOMP_ARG_STR(0);
    if (!plog::get<0>() || !plog::get<0>()->checkSeverity(py_log_severity)) {
        ;
    } else {
        (*plog::get<0>()) += plog::Record(
            py_log_severity, py_log_func.c_str(), py_log_line_number, py_log_file_name.c_str(), reinterpret_cast<void*>(0), 0
        ).ref().printf("%s", log_message.c_str());
    };
}

