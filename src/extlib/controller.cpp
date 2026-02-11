#include "controller.hpp"
#include "embed_handler.hpp"

// Not entirely certain it makes sense for either of these helper functions to
// be part of the class definition. They're used in initializing the interpreter 
// and then never again.
static std::string path_to_string_utf8(const std::filesystem::path& path) {
    std::u8string path_u8string = path.u8string();
    std::string to_escape{ reinterpret_cast<const char*>(path_u8string.c_str()), path_u8string.size() };

    std::string ret{};
    ret.reserve(to_escape.size());
    for (char c : to_escape) {
        // Escape backslashes
        if (c == '\\') {
            ret += '\\';
        }
        ret += c;
    }
    return ret;
}

static void py_preinit_add_search_path(PyConfig* config, fs::path path) {
    PyStatus status;

    wchar_t* pathstr = nullptr;
    status = PyConfig_SetBytesString(config, &pathstr, path_to_string_utf8(path).c_str());
    PyWideStringList_Append(&config->module_search_paths, pathstr);
    PLOGI.printf("'%s' added to Python module search path", path.string().c_str());

    PyMem_RawFree(pathstr);
}

// ======================================  Handle Control: ====================================== 
PyInterpreterController::PyInterpreterController(plog::Severity log_severity, bool log_to_file, fs::path mod_dir, std::queue<fs::path>* registered_nrms) {
    // Initialize Logging
    fs::path file_appender_path = fs::path(mod_dir).parent_path().append("REPY.log");
    log = &plog::init((plog::Severity)log_severity);

    if (log_to_file) {
        file_appender = new plog::RollingFileAppender<plog::TxtFormatter>(path_to_string_utf8(file_appender_path).c_str());
        log->addAppender(file_appender);
    }
    
    console_appender = new plog::ColorConsoleAppender<plog::TxtFormatter>(plog::OutputStream::streamStdOut);
    log->addAppender(console_appender);

    // Setting up python directory:
    fs::path py_dir = fs::path(mod_dir).parent_path().append(PYTHON_VERSION_STR);
    fs::path stdlib_dir = fs::path(py_dir).append(PYTHON_VERSION_STR "_DLLs");
    fs::path stdlib_archive = fs::path(py_dir).append(PYTHON_VERSION_STR ".zip");
    if (!fs::exists(py_dir)) {
        fs::create_directories(py_dir);
    }
    extract_python_stdlib(stdlib_archive);

#ifdef _WIN32
    setup_python_stdlib_dlls(mod_dir, stdlib_dir);
#endif

    // Configuring and Initializing the Interpreter
    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);
    Py_PreInitialize(&preconfig);

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    PyConfig_SetBytesString(&config, &config.program_name, PYTHON_PROGRAM_NAME);

    py_preinit_add_search_path(&config, stdlib_archive);
    py_preinit_add_search_path(&config, stdlib_dir);
    py_preinit_add_search_path(&config, mod_dir);
    py_preinit_add_search_path(&config, py_dir);
    if (registered_nrms != NULL) {
        while (!registered_nrms->empty()) {
            py_preinit_add_search_path(&config, registered_nrms->front());
            registered_nrms->pop();
        }
    }
    config.module_search_paths_set = 1;
    config.parse_argv = 0;
    config.install_signal_handlers = true;

    py::initialize_interpreter(&config, 0, NULL, false); 
    PLOGI << "-> Python interpreter initialized";

    PySubController* main_interp = new PySubController(PYTHON_MAIN_INTERPRETER_HANDLE);
    subinterpreters.push_back(main_interp);

    // Allow other threads to have the GIL.
    py_main_thread = PyEval_SaveThread();
}

PyInterpreterController::~PyInterpreterController() {
    // Restores the GIL to this thread.
    PyEval_RestoreThread(py_main_thread);
    // Release all handles
    py_objects_smap.del_all();

    // End all subinterpreters:
    for (int i = 0; i < subinterpreters.size(); i++) {
        delete subinterpreters.at(i);
    }

    PLOGI << "-> Python interpreter shutdown";
}

REPY_InterpreterHandle PyInterpreterController::create_subcontroller() {
    REPY_InterpreterHandle retVal = subinterpreters.size();
    PySubController* main_interp = new PySubController(retVal);
    subinterpreters.push_back(main_interp);

    PLOGI.printf("Created new subinterpreter with a handle of %u", retVal);

    return retVal;
}

REPY_InterpreterHandle PyInterpreterController::get_current_subcontroller_handle() {
    if (subinterp_handle_stack.empty()) {
        PLOGF.printf("No interpreter selected. Make sure you are using REPY_PushInterpreter and REPY_PopInterpreter correctly");
    }
    assert(!subinterp_handle_stack.empty());
    
    return subinterp_handle_stack.top();
}

PySubController* PyInterpreterController::get_current_subcontroller() {
    return subinterpreters.at(get_current_subcontroller_handle());
}

void PyInterpreterController::push_subcontroller_handle(REPY_InterpreterHandle handle) {
    subinterp_handle_stack.push(handle);
}

void PyInterpreterController::pop_subcontroller_handle() {
    subinterp_handle_stack.pop();
}

REPY_Handle PyInterpreterController::create_handle(py::object* obj) {
    REPY_InterpreterHandle interp_handle = get_current_subcontroller_handle();
    REPY_Handle new_handle = py_objects_smap.add(obj, interp_handle);

    PLOGD.printf("-> REPY_Handle 0x%08X created on interpreter %u", new_handle, interp_handle);
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(*obj).cast<std::u8string>();
        PLOGV.printf("-> Handle %08X: %s", new_handle, repr_str.c_str());
    }
    return new_handle;
}

REPY_InterpreterHandle PyInterpreterController::get_py_object_interpreter(REPY_Handle handle) {
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = py_objects_smap.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    return entry->interp_index;
}

py::object* PyInterpreterController::get_py_object(REPY_Handle handle) {
    REPY_InterpreterHandle current_interp_index = get_current_subcontroller_handle();

    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = py_objects_smap.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    if (entry->interp_index != current_interp_index) {
        PLOGW.printf("REPY_Handle 0x%08X: accessing an interpreter %u object while interpreter %u is active", handle, entry->interp_index, current_interp_index);
    } 
    
    if (entry->is_single_use) {
        suh_release_queue.push(handle);
        PLOGD.printf("-> REPY_Handle 0x%08X from interpreter %u accessed (SUH)", handle, entry->interp_index);
    } else {
        PLOGD.printf("-> REPY_Handle 0x%08X from interpreter %u accessed", handle, entry->interp_index);
    }
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(entry->py_object).cast<std::u8string>();
        PLOGV.printf("Handle 0x%08X: %s", handle, repr_str.c_str());
    }
    return &entry->py_object;
    
}

bool PyInterpreterController::is_valid_handle(REPY_Handle handle) {
    return py_objects_smap.has(handle);
}

bool PyInterpreterController::get_handle_suh(REPY_Handle handle) {
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = py_objects_smap.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    return entry->is_single_use;
}

void PyInterpreterController::set_handle_suh(REPY_Handle handle, bool is_single_use) {
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = py_objects_smap.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    entry->is_single_use = is_single_use;
    PLOGD.printf("-> REPY_Handle %08X setting SUH = %i", handle, is_single_use);
}

void PyInterpreterController::release_suh_handles() {
    while (suh_release_queue.size() > 0) {
        REPY_Handle handle = suh_release_queue.front();
        PLOGD.printf("-> REPY_Handle %08X released (SUH)", handle);
        IF_PLOG(plog::verbose) {
            std::u8string repr_str = py::repr(py_objects_smap.get(handle)->py_object).cast<std::u8string>();
            PLOGV.printf("-> Handle %08X: %s", handle, repr_str.c_str());
        }
        suh_release_queue.pop();
        py_objects_smap.del(handle);
    }
}

void PyInterpreterController::release_handle(REPY_Handle handle) {
    PLOGD.printf("-> REPY_Handle %08X released", handle);
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(py_objects_smap.get(handle)->py_object).cast<std::u8string>();
        PLOGV.printf("-> Handle %08X: %s", handle, repr_str.c_str());

    }
    py_objects_smap.del(handle);
}

py::function PyInterpreterController::py_compile() {
    return get_current_subcontroller()->py_compile();
}

py::function PyInterpreterController::py_exec() {
    return get_current_subcontroller()->py_exec();
}

py::function PyInterpreterController::py_eval() {
    return get_current_subcontroller()->py_eval();
}

py::function PyInterpreterController::py_next() {
    return get_current_subcontroller()->py_next();
}

py::object PyInterpreterController::py_stop_iteration_type() {
    return get_current_subcontroller()->py_stop_iteration_type();
}

py::module_ PyInterpreterController::construct_module(std::string module_name, std::string module_code, bool add_to_sys) {
    py::gil_scoped_acquire gil;
    
    auto types = py::module_::import("types");
    auto new_mod = types.attr("ModuleType")(module_name);

    try {
        py::exec(module_code, new_mod.attr("__dict__"));
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        return py::none();
    }

    if (add_to_sys) {
        auto sys = py::module_::import("sys");
        py::dict sys_modules = sys.attr("modules");
        sys_modules[module_name.c_str()] = new_mod;
    }

    return new_mod;
};

// Error Stuff
bool PyInterpreterController::is_error_set() {
    return get_current_subcontroller()->is_error_set();
}

void PyInterpreterController::handle_exception(py::error_already_set* e) {
    get_current_subcontroller()->handle_exception(e);
}

REPY_Handle PyInterpreterController::get_py_error_type_handle() {
    py::object retVal = get_current_subcontroller()->get_py_error_type();
    return create_handle(&retVal);
}

REPY_Handle PyInterpreterController::get_py_error_trace_handle() {
    py::object retVal = get_current_subcontroller()->get_py_error_trace();
    return create_handle(&retVal);
}

REPY_Handle PyInterpreterController::get_py_error_value_handle() {
    py::object retVal = get_current_subcontroller()->get_py_error_value();
    return create_handle(&retVal);
}

void PyInterpreterController::clear_py_error() {
    get_current_subcontroller()->clear_py_error();
}

REPY_Handle PyInterpreterController::get_zipfile_from_path(std::u8string filepath) {
    py::object retVal = get_current_subcontroller()->get_zipfile_from_path(filepath);
    return create_handle(&retVal);
}

uint8_t* PyInterpreterController::get_rdram() {
    return rdram;
}

void PyInterpreterController::set_rdram(uint8_t* p_rdram) {
    rdram = p_rdram;
}

std::unique_ptr<PyInterpreterController> controller = NULL;
