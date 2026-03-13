#pragma once
#include <atomic>
#include "globals.hpp"

class GlobalInterpreterController;

// Holds references to interpreter-specific Python objects needed for the API.
class GlobalInterpreterController {
public:
    GlobalInterpreterController() = delete;
    GlobalInterpreterController(REPY_InterpreterIndex index);
    ~GlobalInterpreterController();
    
    REPY_InterpreterIndex get_index();
    py::subinterpreter* get_subinterp();

    bool get_auto_disarm();
    void set_auto_disarm(bool val);

    py::function py_compile();
    py::function py_exec();
    py::function py_eval();
    py::function py_next();
    py::object py_stop_iteration_type();
    py::object get_zipfile_from_path(std::u8string filepath);

private:
    // No atomic here. Only gets read after initialization.
    REPY_InterpreterIndex _index;   
    py::subinterpreter _subinterp;

    std::atomic<bool> _auto_disarm;
    py::function _py_compile;
    py::function _py_exec;
    py::function _py_eval;
    py::function _py_next;
    py::object _py_stop_iteration_type;
    py::module_ _py_zipfile_module;
    py::object _py_zipfile_class;

    void init_py_objects();
};