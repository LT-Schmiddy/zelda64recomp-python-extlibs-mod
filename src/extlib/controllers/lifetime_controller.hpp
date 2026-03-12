#pragma once
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
    LifetimeController(plog::Severity severity, bool log_to_file, fs::path mod_dir, std::queue<fs::path>* registered_nrms);
    ~LifetimeController();

private:
    std::unique_ptr<GlobalRootController> _global = NULL;
    std::unordered_map<std::thread::id, ThreadRootController> _threads;

    plog::RollingFileAppender<plog::TxtFormatter>* _file_appender = NULL;
    plog::ColorConsoleAppender<plog::TxtFormatter>* _console_appender = NULL;
    plog::Logger<0>* _log = NULL;
};