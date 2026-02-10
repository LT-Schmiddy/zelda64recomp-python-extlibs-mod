#pragma once
#include <memory>
#include <queue>
#include "globals.hpp"
#include "lib_recomp.hpp"
#include "pyobject_slotmap.hpp"




class PySubinterpreterController {
public:
    PySubinterpreterController(REPY_SubinterpHandle p_index);
    ~PySubinterpreterController();

    REPY_SubinterpHandle get_index();
    bool is_active();

    void activate();
    void deactivate();

    py::function compile();
    py::function exec();
    py::function eval();
    py::function next();

    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    py::object get_py_error_type_handle();
    py::object get_py_error_trace_handle();
    py::object get_py_error_value_handle();
    void clear_py_error();

    py::object get_zipfile_from_path(std::u8string filepath);

private:

    py::function _py_compile;
    py::function _py_exec;
    py::function _py_eval;
    py::function _py_next;

    py::module_ _py_zipfile_module;
    py::object _py_zipfile_class;
    py::object _py_stop_iteration_type;
    
    bool _is_py_error_set = false;
    py::object _last_error_type = py::none();
    py::object _last_error_trace = py::none();
    py::object _last_error_value = py::none();

    void init_py_objects();

    REPY_SubinterpHandle _index;    
    py::subinterpreter _subinterp;
    py::subinterpreter_scoped_activate* _scope = NULL;

};