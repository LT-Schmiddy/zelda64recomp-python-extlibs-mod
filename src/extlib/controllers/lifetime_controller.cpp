#include "lifetime_controller.hpp"

#include "embed_handler.hpp"

#include <exception>

// Not entirely certain it makes sense for either of these helper functions to
// be part of the class definition. They're used in initializing the interpreter 
// and then never again.
static std::string path_to_string_utf8(const std::filesystem::path& path) {
    ZoneScoped;
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
    ZoneScoped;
    PyStatus status;

    wchar_t* pathstr = nullptr;
    status = PyConfig_SetBytesString(config, &pathstr, path_to_string_utf8(path).c_str());
    PyWideStringList_Append(&config->module_search_paths, pathstr);
    PLOGI.printf("'%s' added to Python module search path", path.string().c_str());

    PyMem_RawFree(pathstr);
}

// ======================================  Handle Control: ====================================== 
LifetimeController::LifetimeController(uint8_t* rdram, plog::Severity log_severity, bool log_to_file, fs::path mod_dir, std::queue<fs::path>* registered_nrms) {
    ZoneScoped;

    // Initialize Logging
    fs::path file_appender_path = fs::path(mod_dir).parent_path().append("REPY.log");
    _log = &plog::init((plog::Severity)log_severity);

    if (log_to_file) {
        _file_appender = new plog::RollingFileAppender<plog::TxtFormatter>(path_to_string_utf8(file_appender_path).c_str());
        _log->addAppender(_file_appender);
    }
    
    _console_appender = new plog::ColorConsoleAppender<plog::TxtFormatter>(plog::OutputStream::streamStdOut);
    _log->addAppender(_console_appender);

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

    // {

    //     py::gil_scoped_acquire gil; 

    // }
    _global_root = new GlobalRootController(rdram);
}

LifetimeController::~LifetimeController() {
    ZoneScoped;

    PLOGI << "REPY: Lifetime Controller Finalized";
}


GlobalRootController* LifetimeController::get_global_root_controller() {
    ZoneScoped;
    return _global_root;
}

ThreadRootController* LifetimeController::get_thread_root_controller(std::thread::id thread_id) {
    ZoneScoped;
    std::lock_guard guard(_thread_root_mutex);

    if (!_thread_root.contains(thread_id)) {
        ThreadRootController* retVal = _global_root->create_thread_root_controller(thread_id);
        _thread_root.insert(std::make_pair(thread_id, retVal));
        return retVal;
    }

    return _thread_root.at(thread_id);
}

ThreadRootController* LifetimeController::get_current_thread_root_controller() {
    ZoneScoped;
    return get_thread_root_controller(std::this_thread::get_id());
}