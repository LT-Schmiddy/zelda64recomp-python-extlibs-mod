#include "thread_interpreter_controller.hpp"

ThreadInterpreterController::ThreadInterpreterController(GlobalRootController* global_root, GlobalInterpreterController* global_interp) {
    ZoneScoped;
    _global_root = global_root;
    _global_interp = global_interp;

    PLOGI.printf("Creating ThreadInterpreterController (Interpreter %u, Thread %llu)", _global_interp->get_index(), std::this_thread::get_id());
    _last_error_type = py::none();
    _last_error_trace = py::none();
    _last_error_value = py::none();
}

ThreadInterpreterController::~ThreadInterpreterController() {
    ZoneScoped;

    if (_is_active) {
        deactivate();
    }
    PLOGI.printf("Creating ThreadInterpreterController (Interpreter %u)", _global_interp->get_index());
}

// Activation:
void ThreadInterpreterController::activate() {
    ZoneScoped;
    assert(!_is_active);
    REPY_InterpreterIndex _index = _global_interp->get_index();
    _is_active = true;

    if (_index != 0) {
        _scope = new py::subinterpreter_scoped_activate(*(_global_interp->get_subinterp()));
    }

    _gil = new py::gil_scoped_acquire();

    if (_index != 0) {
        PLOGD.printf("Activated Python interpreter %u", _index);
    } else {
        PLOGD.printf("Activated Python interpreter %u (main interpreter)", _index);
    }
}

void ThreadInterpreterController::deactivate() {
    ZoneScoped;
    assert(_is_active);
    REPY_InterpreterIndex _index = _global_interp->get_index();
    _is_active = false;

    delete _gil;
    _gil = nullptr;

    if (_index != 0) {
        delete _scope;
        _scope = nullptr;
        PLOGD.printf("Deactivated Python interpreter %u", _index);
    } else {
        PLOGD.printf("Deactivated Python interpreter %u (main interpreter)", _index);
    }
}

bool ThreadInterpreterController::is_active() {
    ZoneScoped;
    return _is_active;
}

// Accessing Python Objects:
bool ThreadInterpreterController::get_auto_disarm() {
    ZoneScoped;
    return _global_interp->get_auto_disarm();
}

void ThreadInterpreterController::set_auto_disarm(bool val) {
    ZoneScoped;
    _global_interp->set_auto_disarm(val);
}

py::function ThreadInterpreterController::py_compile() {
    ZoneScoped;
    return _global_interp->py_compile();
}

py::function ThreadInterpreterController::py_exec() {
    ZoneScoped;
    return _global_interp->py_exec();
}

py::function ThreadInterpreterController::py_eval() {
    ZoneScoped;
    return _global_interp->py_eval();
}

py::function ThreadInterpreterController::py_next() {
    ZoneScoped;
    return _global_interp->py_next();
}

py::object ThreadInterpreterController::py_stop_iteration_type() {
    ZoneScoped;
    return _global_interp->py_stop_iteration_type();
}

py::object ThreadInterpreterController::get_zipfile_from_path(std::u8string filepath) {
    ZoneScoped;
    return _global_interp->get_zipfile_from_path(filepath);
}

// Error Handling:
// Error Stuff
bool ThreadInterpreterController::is_error_set() {
    ZoneScoped;
    return _is_py_error_set;
}

void ThreadInterpreterController::handle_exception(py::error_already_set* e) {
    ZoneScoped;
    _is_py_error_set = true;

    PLOGE << e->what();
    _last_error_type = e->type();
    _last_error_trace = e->trace();
    _last_error_value = e->value();
}

py::object ThreadInterpreterController::get_py_error_type() {
    ZoneScoped;
    return _last_error_type;
}

py::object ThreadInterpreterController::get_py_error_trace() {
    ZoneScoped;
    return _last_error_trace;
}

py::object ThreadInterpreterController::get_py_error_value() {
    ZoneScoped;
    return _last_error_value;
}

void ThreadInterpreterController::clear_py_error() {
    ZoneScoped;
    _is_py_error_set = false;
    _last_error_type = py::none();
    _last_error_trace = py::none();
    _last_error_value = py::none();
}