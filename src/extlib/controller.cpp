#include "controller.hpp"
// ======================================  Handle Control: ====================================== 

PyInterpreterController::PyInterpreterController(plog::Severity severity, fs::path mod_dir) {
    fs::path mod_dir_DLLs = fs::path(mod_dir).append("DLLs");
    fs::path mod_dir_Lib = fs::path(mod_dir).append("Lib");
    fs::path mod_dir_site = fs::path(mod_dir_Lib).append("site-packages");

    file_appender = new plog::RollingFileAppender<plog::TxtFormatter>("REPY.log");
    console_appender = new plog::ColorConsoleAppender<plog::TxtFormatter>(plog::OutputStream::streamStdOut);
    log = &plog::init((plog::Severity)severity);
    log->addAppender(file_appender);
    log->addAppender(console_appender);

    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);
    Py_PreInitialize(&preconfig);

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.parse_argv = 0;
    config.install_signal_handlers = true;

    py::initialize_interpreter(&config); 
    PLOGI << "-> Python Interpreter Parent: INIT";

    auto sys = py::module_::import("sys");
    auto sys_path = sys.attr("path");

    sys_path.attr("clear")();
    sys_path.attr("append")(mod_dir.string());
    sys_path.attr("append")(mod_dir_DLLs.string());
    sys_path.attr("append")(mod_dir_Lib.string());
    sys_path.attr("append")(mod_dir_site.string());

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
        py::gil_scoped_acquire gil;
        py_objects.clear();
    }
    PyEval_RestoreThread(py_main_thread);
    PLOGI << "-> Python Interpreter Parent: DEINIT";
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

    PLOGD.printf("-> PyObjectHandle %i Created", new_handle);
    return new_handle;
}

py::object PyInterpreterController::get_py_object(PyObjectHandle handle) {
    PyObjectHandleEntry* entry = &py_objects.at(handle);
    py::object retVal = entry->py_object;
    if (entry->is_single_use) {
        py_objects.erase(handle);
        PLOGD.printf("-> PyObjectHandle %i Accessed and Released (SUH)", handle);
    } else {
        PLOGD.printf("-> PyObjectHandle %i Accessed", handle);
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
    PLOGD.printf("-> PyObjectHandle %i Setting SUH = %i", handle, is_single_use);
}


void PyInterpreterController::release_handle(PyObjectHandle handle) {
    py_objects.erase(handle);
    PLOGD.printf("-> PyObjectHandle %i Released", handle);
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
    // Technically deprecated, but I still wanna call it.
    e->clear();
}

PyObjectHandle PyInterpreterController::get_py_error_type_handle() {
    if(!is_py_error_set) {
        return 0;
    }

    return create_handle(last_error_type);
}

PyObjectHandle PyInterpreterController::get_py_error_trace_handle() {
    if(!is_py_error_set) {
        return 0;
    }

    return create_handle(last_error_trace);
}

PyObjectHandle PyInterpreterController::get_py_error_value_handle() {
    if(!is_py_error_set) {
        return 0;
    }

    return create_handle(last_error_value);
}

void PyInterpreterController::clear_py_error() {
    is_py_error_set = false;
    last_error_type = py::none();
    last_error_trace = py::none();
    last_error_value = py::none();
}

int PyInterpreterController::random_in_range(int low, int high) {
    return rand() % (high - low + 1) + low;
}

void PyInterpreterController::set_rdram(uint8_t* p_rdram) {
    rdram = p_rdram;
}

std::shared_ptr<PyInterpreterController> controller = NULL;
