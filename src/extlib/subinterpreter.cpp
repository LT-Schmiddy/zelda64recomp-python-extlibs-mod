#include "subinterpreter.hpp"
#include "controller.hpp"

PySubController::PySubController(REPY_InterpreterHandle p_index) {
    ZoneScoped
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
    ZoneScoped
}

REPY_InterpreterHandle PySubController::get_index() {
    ZoneScoped
    return _index;
}

py::subinterpreter* PySubController::get_subinterpreter() {
    ZoneScoped
    if (_index != 0) {
        return &_subinterp;
    } else {
        return NULL;
    }
}

py::function PySubController::py_compile() {
    ZoneScoped
    return _py_compile;
}

py::function PySubController::py_exec() {
    ZoneScoped
    return _py_exec;
}

py::function PySubController::py_eval() {
    ZoneScoped
    return _py_eval;
}

py::function PySubController::py_next() {
    ZoneScoped
    return _py_next;
}

py::object PySubController::py_stop_iteration_type() {
    ZoneScoped
    return _py_stop_iteration_type;
}

// Error Stuff
bool PySubController::is_error_set() {
    ZoneScoped
    return _is_py_error_set;
}

void PySubController::handle_exception(py::error_already_set* e) {
    ZoneScoped
    _is_py_error_set = true;

    PLOGE << e->what();
    _last_error_type = e->type();
    _last_error_trace = e->trace();
    _last_error_value = e->value();
}

py::object PySubController::get_py_error_type() {
    ZoneScoped
    return _last_error_type;
}

py::object PySubController::get_py_error_trace() {
    ZoneScoped
    return _last_error_trace;
}

py::object PySubController::get_py_error_value() {
    ZoneScoped
    return _last_error_value;
}

void PySubController::clear_py_error() {
    ZoneScoped
    _is_py_error_set = false;
    _last_error_type = py::none();
    _last_error_trace = py::none();
    _last_error_value = py::none();
}

py::object PySubController::get_zipfile_from_path(std::u8string filepath) {
    ZoneScoped
    return _py_zipfile_class(py::str(filepath));
}

void PySubController::init_py_objects() {
    ZoneScoped
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
    ZoneScoped
    
    _subcontroller = subcontroller;
    assert(subcontroller != NULL);

    _subinterp = _subcontroller->get_subinterpreter();
    if (_subinterp != NULL) {
        _scope = new py::subinterpreter_scoped_activate(*_subinterp);
        PLOGV.printf("Loaded PySubControllerScope for subcontroller %u", _subcontroller->get_index());
    } else {
        PLOGV.printf("Loaded PySubControllerScope for subcontroller %u (main interpreter)", _subcontroller->get_index());
    }
}

PySubControllerScope::~PySubControllerScope() {
    ZoneScoped
    
    if (_subinterp != NULL) {
        delete _scope;
        PLOGV.printf("Released PySubControllerScope for subcontroller %u", _subcontroller->get_index());
    } else {
        PLOGV.printf("Released PySubControllerScope for subcontroller %u (main interpreter)", _subcontroller->get_index());
    }
}
