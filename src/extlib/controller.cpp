#include "controller.hpp"
#include "embed_handler.hpp"

// This allows for multiple zips to be copied, but only one ended up being used.
// There may be a use case for multiple zips in the future.
std::string path_to_string_utf8(const std::filesystem::path& path) {
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

void py_preinit_add_search_path(PyConfig* config, fs::path path) {
    PyStatus status;

    wchar_t* pathstr = nullptr;
    status = PyConfig_SetBytesString(config, &pathstr, path_to_string_utf8(path).c_str());
    PyWideStringList_Append(&config->module_search_paths, pathstr);
    PyMem_RawFree(pathstr);
}

// ======================================  Handle Control: ====================================== 
PyInterpreterController::PyInterpreterController(plog::Severity severity, fs::path mod_dir) {
    file_appender = new plog::RollingFileAppender<plog::TxtFormatter>("REPY.log");
    console_appender = new plog::ColorConsoleAppender<plog::TxtFormatter>(plog::OutputStream::streamStdOut);
    log = &plog::init((plog::Severity)severity);
    log->addAppender(file_appender);
    log->addAppender(console_appender);

    fs::path mod_dir_Lib = fs::path(mod_dir).append("python313.zip");
    fs::path mod_dir_DLLs = fs::path(mod_dir);
    // fs::path mod_dir_nrm = fs::path(mod_dir).append("MM_Recomp_External_Python_API.nrm");
    extract_python_stdlib(mod_dir_Lib);
    setup_python_stdlib_dlls(mod_dir);

    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);
    Py_PreInitialize(&preconfig);

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    PyConfig_SetBytesString(&config, &config.program_name, "Zelda64Recompiled");
    
    py_preinit_add_search_path(&config, mod_dir_Lib);
    py_preinit_add_search_path(&config, mod_dir_DLLs);
    // py_preinit_add_search_path(&config, mod_dir_nrm);
    // py_preinit_add_search_path(&config, mod_dir_site);

    config.module_search_paths_set = 1;

    config.parse_argv = 0;
    config.install_signal_handlers = true;

    py::initialize_interpreter(&config); 
    PLOGI << "-> Python Interpreter Parent: INIT";

    auto sys = py::module_::import("sys");
    auto sys_path = sys.attr("path");

    py::print(sys_path);

    auto builtins = py::module_::import("builtins");
    py_compile = builtins.attr("compile");
    py_exec = builtins.attr("exec");
    py_eval = builtins.attr("eval");
    
    // Allow other threads to have the GIL.
    py_main_thread = PyEval_SaveThread();
};

PyInterpreterController::~PyInterpreterController() {
    // Restores the GIL to this thread.

    // The DLL unloading process seems to clean up the interpreter on it's own,
    // But it doesn't seem to like it when we have handles left over.
    {
        // py::gil_scoped_acquire gil;
        // py_objects.clear();
    }
    PyEval_RestoreThread(py_main_thread);
    py_objects.clear();
    PLOGI << "-> Python Interpreter Parent: DEINIT";
}

REPY_Handle PyInterpreterController::get_new_handle_value() {
    py::gil_scoped_acquire gil;
    while (py_objects.contains(next_handle_val) || next_handle_val == 0) {
        next_handle_val++;
    }
    return next_handle_val++;
}

REPY_Handle PyInterpreterController::create_handle_and_steal(py::object* obj) {
    py::gil_scoped_acquire gil;
    REPY_Handle new_handle = get_new_handle_value();
    py_objects.insert({new_handle, {py::reinterpret_steal<py::object>(*obj), false}});

    PLOGD.printf("-> REPY_Handle %i Created", new_handle);
    return new_handle;
}

REPY_Handle PyInterpreterController::create_handle(py::object* obj) {
    py::gil_scoped_acquire gil;
    REPY_Handle new_handle = get_new_handle_value();
    // py::object in_obj = (*obj);
    py_objects.insert({new_handle, {(*obj), false}});

    PLOGD.printf("-> REPY_Handle %i Created", new_handle);
    return new_handle;
}

py::object* PyInterpreterController::get_py_object(REPY_Handle handle) {
    REPY_HandleEntry* entry = &py_objects.at(handle);
    if (entry->is_single_use) {
        suh_release_queue.push(handle);
        PLOGD.printf("-> REPY_Handle %i Accessed (SUH)", handle);
    } else {
        PLOGD.printf("-> REPY_Handle %i Accessed", handle);
    }

    return &entry->py_object;
}

bool PyInterpreterController::is_valid_handle(REPY_Handle handle) {
    return py_objects.contains(handle);
}


bool PyInterpreterController::get_handle_suh(REPY_Handle handle) {
    REPY_HandleEntry* entry = &py_objects.at(handle);
    return entry->is_single_use;
}

void PyInterpreterController::set_handle_suh(REPY_Handle handle, bool is_single_use) {
    REPY_HandleEntry* entry = &py_objects.at(handle);
    entry->is_single_use = is_single_use;
    PLOGD.printf("-> REPY_Handle %i Setting SUH = %i", handle, is_single_use);
}

void PyInterpreterController::release_suh_handles() {
    while (suh_release_queue.size() > 0) {
        REPY_Handle handle = suh_release_queue.front();
        suh_release_queue.pop();
        py_objects.erase(handle);
        PLOGD.printf("-> REPY_Handle %i Released (SUH)", handle);
    }
}

void PyInterpreterController::release_handle(REPY_Handle handle) {
    py_objects.erase(handle);
    PLOGD.printf("-> REPY_Handle %i Released", handle);
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
    if(!is_py_error_set) {
        return 0;
    }

    return create_handle(&last_error_type);
}

REPY_Handle PyInterpreterController::get_py_error_trace_handle() {
    if(!is_py_error_set) {
        return 0;
    }

    return create_handle(&last_error_trace);
}

REPY_Handle PyInterpreterController::get_py_error_value_handle() {
    if(!is_py_error_set) {
        return 0;
    }

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
