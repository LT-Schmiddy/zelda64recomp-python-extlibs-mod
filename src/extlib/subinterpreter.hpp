#pragma once
#include <memory>
#include <queue>
#include "globals.hpp"
#include "lib_recomp.hpp"
#include "pyobject_slotmap.hpp"

class PySubController {
public:
    PySubController(REPY_InterpreterIndex p_index);
    ~PySubController();

    REPY_InterpreterIndex get_index();
    py::subinterpreter* get_subinterpreter();

    void activate();
    void deactivate();
    bool is_active();

    bool get_auto_disarm();
    void set_auto_disarm(bool val);

    py::function py_compile();
    py::function py_exec();
    py::function py_eval();
    py::function py_next();
    py::object py_stop_iteration_type();

    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    py::object get_py_error_type();
    py::object get_py_error_trace();
    py::object get_py_error_value();
    void clear_py_error();

    py::object get_zipfile_from_path(std::u8string filepath);

private:
    REPY_InterpreterIndex _index;    
    py::subinterpreter _subinterp;
    py::gil_scoped_acquire* _gil = NULL;
    py::subinterpreter_scoped_activate* _scope = NULL;
    bool _is_active = false;

    bool _auto_disarm = false;

    py::function _py_compile;
    py::function _py_exec;
    py::function _py_eval;
    py::function _py_next;
    py::object _py_stop_iteration_type;

    py::module_ _py_zipfile_module;
    py::object _py_zipfile_class;
    
    
    bool _is_py_error_set = false;
    py::object _last_error_type = py::none();
    py::object _last_error_trace = py::none();
    py::object _last_error_value = py::none();

    void init_py_objects();


};