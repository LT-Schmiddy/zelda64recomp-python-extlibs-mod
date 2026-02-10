#include "subinterpreter.hpp"

PySubinterpreterController::PySubinterpreterController(REPY_SubinterpHandle p_index) {
    _index = p_index;

    if (_index) {
        _subinterp = py::subinterpreter::create();
        py::subinterpreter_scoped_activate activate(_subinterp);
        init_py_objects();
    } else {
        // id == 0 is a special case, referencing the global interpreter.
        init_py_objects();
    }
}

PySubinterpreterController::~PySubinterpreterController() {
    if (_scope != NULL) {
        deactivate();
    };
}

uint32_t PySubinterpreterController::get_index() {
    return _index;
}

bool PySubinterpreterController::is_active() {
    return _scope != NULL;
}

void PySubinterpreterController::activate() {
    _scope = new py::subinterpreter_scoped_activate(_subinterp);
}

void PySubinterpreterController::deactivate() {
    delete _scope;
    _scope = NULL;
}

py::function PySubinterpreterController::compile() {
    return _py_compile;
}
py::function PySubinterpreterController::exec() {
    return _py_exec;
}
py::function PySubinterpreterController::eval() {
    return _py_eval;
}
py::function PySubinterpreterController::next() {
    return _py_next;
}

// Error Stuff
bool PySubinterpreterController::is_error_set() {
    return _is_py_error_set;
}

void PySubinterpreterController::handle_exception(py::error_already_set* e) {
    _is_py_error_set = true;

    PLOGE << e->what();
    _last_error_type = e->type();
    _last_error_trace = e->trace();
    _last_error_value = e->value();
}

py::object PySubinterpreterController::get_py_error_type() {
    return _last_error_type;
}

py::object PySubinterpreterController::get_py_error_trace_handle() {
    return _last_error_trace;
}

py::object PySubinterpreterController::get_py_error_value_handle() {
    return _last_error_value;
}

void PySubinterpreterController::clear_py_error() {
    _is_py_error_set = false;
    _last_error_type = py::none();
    _last_error_trace = py::none();
    _last_error_value = py::none();
}

py::object PySubinterpreterController::get_zipfile_from_path(std::u8string filepath) {
    return _py_zipfile_class(py::str(filepath));
}

void PySubinterpreterController::init_py_objects() {
    auto builtins = py::module_::import("builtins");
    _py_compile = builtins.attr("compile");
    _py_exec = builtins.attr("exec");
    _py_eval = builtins.attr("eval");
    _py_next = builtins.attr("next");

    _py_zipfile_module = py::module_::import("zipfile");
    _py_zipfile_class = _py_zipfile_module.attr("ZipFile");
    _py_stop_iteration_type = py::eval("StopIteration");
}