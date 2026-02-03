#include "controller.hpp"
#include "embed_handler.hpp"

// This allows for multiple zips to be copied, but only one ended up being used.
// There may be a use case for multiple zips in the future.
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

    // Setting up Stdlib
    fs::path stdlib_dir = fs::path(mod_dir).append(PYTHON_VERSION_STR "_DLLs");
    fs::path stdlib_archive = fs::path(mod_dir).append(PYTHON_VERSION_STR ".zip");
    extract_python_stdlib(stdlib_archive);
    setup_python_stdlib_dlls(mod_dir, stdlib_dir);

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
    if (registered_nrms != NULL) {
        while (!registered_nrms->empty()) {
            py_preinit_add_search_path(&config, registered_nrms->front());
            registered_nrms->pop();
        }
    }
    config.module_search_paths_set = 1;

    config.parse_argv = 0;
    config.install_signal_handlers = true;

    py::initialize_interpreter(&config); 
    PLOGI << "-> Python interpreter initialized";

    auto builtins = py::module_::import("builtins");
    py_compile = builtins.attr("compile");
    py_exec = builtins.attr("exec");
    py_eval = builtins.attr("eval");
    py_next = builtins.attr("next");

    py_stop_iteration_type = py::eval("StopIteration");

    // Allow other threads to have the GIL.
    py_main_thread = PyEval_SaveThread();
};

PyInterpreterController::~PyInterpreterController() {
    // Restores the GIL to this thread.

    PyEval_RestoreThread(py_main_thread);
    py_objects_smap.del_all();

    PLOGI << "-> Python interpreter shutdown";
}

REPY_Handle PyInterpreterController::create_handle(py::object* obj) {
    REPY_Handle new_handle;
    new_handle = py_objects_smap.add(obj);

    PLOGD.printf("-> REPY_Handle 0x%08X created", new_handle);
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(*obj).cast<std::u8string>();
        PLOGV.printf("-> Handle %08X: %s", new_handle, repr_str.c_str());
    }
    return new_handle;
}

py::object* PyInterpreterController::get_py_object(REPY_Handle handle) {
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = py_objects_smap.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    if (entry->is_single_use) {
        suh_release_queue.push(handle);
        PLOGD.printf("-> REPY_Handle 0x%08X accessed (SUH)", handle);
    } else {
        PLOGD.printf("-> REPY_Handle 0x%08X accessed", handle);
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
    return is_py_error_set;
}

void PyInterpreterController::handle_exception(py::error_already_set* e) {
    is_py_error_set = true;

    PLOGE << e->what();
    last_error_type = e->type();
    last_error_trace = e->trace();
    last_error_value = e->value();
}

REPY_Handle PyInterpreterController::get_py_error_type_handle() {
    return create_handle(&last_error_type);
}

REPY_Handle PyInterpreterController::get_py_error_trace_handle() {
    return create_handle(&last_error_trace);
}

REPY_Handle PyInterpreterController::get_py_error_value_handle() {
    return create_handle(&last_error_value);
}

void PyInterpreterController::clear_py_error() {
    is_py_error_set = false;
    last_error_type = py::none();
    last_error_trace = py::none();
    last_error_value = py::none();
}

void PyInterpreterController::set_rdram(uint8_t* p_rdram) {
    rdram = p_rdram;
}

std::shared_ptr<PyInterpreterController> controller = NULL;
