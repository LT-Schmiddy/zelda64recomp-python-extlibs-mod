#pragma once
#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <unordered_map>
#include "globals.hpp"

#include <plog/Formatters/TxtFormatter.h>// Step1: include the headers
#include <plog/Appenders/ColorConsoleAppender.h>// Step1: include the headers
#include <plog/Initializers/RollingFileInitializer.h>

#include "global_root_controller.hpp"
#include "global_interpreter_controller.hpp"
#include "thread_root_controller.hpp"
#include "thread_interpreter_controller.hpp"


// Responsible for managing the lifetime of the Python interpreter,
// plog, and the root controllers.
class LifetimeController {
public:
    LifetimeController() = delete;
    LifetimeController(uint8_t* rdram, plog::Severity log_severity, bool log_to_file, fs::path mod_dir, std::queue<fs::path>* registered_nrms);
    ~LifetimeController();

    GlobalRootController* get_global_root_controller();
    ThreadRootController* get_thread_root_controller(std::thread::id thread_id);
    ThreadRootController* get_current_thread_root_controller();

    void thread_check_exception();

private:
    plog::RollingFileAppender<plog::TxtFormatter>* _file_appender = nullptr;
    plog::ColorConsoleAppender<plog::TxtFormatter>* _console_appender = nullptr;
    plog::Logger<0>* _log = nullptr;

    PyThreadState* _py_main_thread = nullptr;
    GlobalRootController* _global_root = nullptr;
    
    std::mutex _thread_root_mutex;
    std::unordered_map<std::thread::id, ThreadRootController*> _thread_root;
};