#include "subinterpreter.hpp"
#include "controller.hpp"

PySubController::PySubController(REPY_SubcontrollerHandle p_index) {
    _index = p_index;

    if (_index != PYTHON_MAIN_INTERPRETER_HANDLE) {
        _subinterp = py::subinterpreter::create();
        py::subinterpreter_scoped_activate activate(_subinterp);
        init_py_objects();
    } else {
        // id == 0 is a special case, referencing the global interpreter.
        init_py_objects();
    }
}

PySubController::~PySubController() {

}

REPY_SubcontrollerHandle PySubController::get_index() {
    return _index;
}

py::subinterpreter* PySubController::get_subinterpreter() {
    if (_index != 0) {
        return &_subinterp;
    } else {
        return NULL;
    }
}

py::function PySubController::py_compile() {
    return _py_compile;
}

py::function PySubController::py_exec() {
    return _py_exec;
}

py::function PySubController::py_eval() {
    return _py_eval;
}

py::function PySubController::py_next() {
    return _py_next;
}

py::object PySubController::py_stop_iteration_type() {
    return _py_stop_iteration_type;
}

// Error Stuff
bool PySubController::is_error_set() {
    return _is_py_error_set;
}

void PySubController::handle_exception(py::error_already_set* e) {
    _is_py_error_set = true;

    PLOGE << e->what();
    _last_error_type = e->type();
    _last_error_trace = e->trace();
    _last_error_value = e->value();
}

py::object PySubController::get_py_error_type() {
    return _last_error_type;
}

py::object PySubController::get_py_error_trace() {
    return _last_error_trace;
}

py::object PySubController::get_py_error_value() {
    return _last_error_value;
}

void PySubController::clear_py_error() {
    _is_py_error_set = false;
    _last_error_type = py::none();
    _last_error_trace = py::none();
    _last_error_value = py::none();
}

py::object PySubController::get_zipfile_from_path(std::u8string filepath) {
    return _py_zipfile_class(py::str(filepath));
}

void PySubController::init_py_objects() {
    auto builtins = py::module_::import("builtins");
    _py_compile = builtins.attr("compile");
    _py_exec = builtins.attr("exec");
    _py_eval = builtins.attr("eval");
    _py_next = builtins.attr("next");

    _py_zipfile_module = py::module_::import("zipfile");
    _py_zipfile_class = _py_zipfile_module.attr("ZipFile");
    _py_stop_iteration_type = py::eval("StopIteration");
}

// Scope Handling
PySubControllerScope::PySubControllerScope(PySubController* subcontroller) {
    
    _subcontroller = subcontroller;
    assert(subcontroller != NULL);

    _subinterp = _subcontroller->get_subinterpreter();
    if (_subinterp != NULL) {
        _scope = new py::subinterpreter_scoped_activate(*_subinterp);
    }
}

PySubControllerScope::~PySubControllerScope() {
    if (_subinterp != NULL) {
        delete _scope;
    }
}
