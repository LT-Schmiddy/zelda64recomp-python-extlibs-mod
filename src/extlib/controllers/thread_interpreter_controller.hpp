#pragma once
#include "globals.hpp"
#include "global_interpreter_controller.hpp"
#include "global_root_controller.hpp"


// Manages the activation and deactivation of this interpreter on this thread:
// Only used on one thread. No concurrency safety needed.
class ThreadInterpreterController {
public:
    ThreadInterpreterController(GlobalRootController* global_root, GlobalInterpreterController* global_interp);
    ~ThreadInterpreterController();

    bool get_auto_disarm();
    void set_auto_disarm(bool val);

    py::function py_compile();
    py::function py_exec();
    py::function py_eval();
    py::function py_next();
    py::object py_stop_iteration_type();
    py::object get_zipfile_from_path(std::u8string filepath);

    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    py::object get_py_error_type();
    py::object get_py_error_trace();
    py::object get_py_error_value();
    void clear_py_error();

private:
    GlobalRootController* _global_root = nullptr;
    GlobalInterpreterController* _global_interp = nullptr;

    bool _is_py_error_set = false;
    py::object _last_error_type = py::none();
    py::object _last_error_trace = py::none();
    py::object _last_error_value = py::none();
    void init_py_objects();
};