#include "global_interpreter_controller.hpp"

GlobalInterpreterController::GlobalInterpreterController(REPY_InterpreterIndex index) {
    ZoneScoped;
    _index = index;
    _auto_disarm.store(false);
    
    // Capturing critical Python objects:
    if (_index != PYTHON_MAIN_INTERPRETER_HANDLE) {
        _subinterp = py::subinterpreter::create();
        py::subinterpreter_scoped_activate activate(_subinterp);
        init_py_objects();
    } else {
        // id == 0 is a special case, referencing the global interpreter.

        init_py_objects();
    }
}

GlobalInterpreterController::~GlobalInterpreterController() {
    ZoneScoped;
}

REPY_InterpreterIndex GlobalInterpreterController::get_index() {
    ZoneScoped;
    return _index;
}

py::subinterpreter* GlobalInterpreterController::get_subinterp() {
    ZoneScoped;
    return &_subinterp;
}

bool GlobalInterpreterController::get_auto_disarm() {
    ZoneScoped;
    if (!_index) {
        return false;
    }
    return _auto_disarm.load();
}

void GlobalInterpreterController::set_auto_disarm(bool val) {
    ZoneScoped;
    if (!_index) {
        PLOGW.printf("Setting Auto-Disarm on the main interpreter does nothing.");
        return; 
    }
    _auto_disarm.store(val);
    PLOGI.printf("Subinterpreter %i Auto-Disarm set to %u", _auto_disarm.load());
}

py::function GlobalInterpreterController::py_compile() {
    ZoneScoped;
    return _py_compile;
}

py::function GlobalInterpreterController::py_exec() {
    ZoneScoped;
    return _py_exec;
}

py::function GlobalInterpreterController::py_eval() {
    ZoneScoped;
    return _py_eval;
}

py::function GlobalInterpreterController::py_next() {
    ZoneScoped;
    return _py_next;
}

py::object GlobalInterpreterController::py_stop_iteration_type() {
    ZoneScoped;
    return _py_stop_iteration_type;
}

py::object GlobalInterpreterController::get_zipfile_from_path(std::u8string filepath) {
    ZoneScoped;
    return _py_zipfile_class(py::str(filepath));
}

void GlobalInterpreterController::init_py_objects() {
    ZoneScoped;
    // The GIL seems to want to be acquired at least once for pybind11's  GIL/thread
    // management to work correctly. This initialization is always done on the main thread,
    // so we'll do it here. Besides, we should have it for getting these objects anyway, 
    // and can't rely on mod-code interpreter activations yet.
    py::gil_scoped_acquire gil; 
    auto builtins = py::module_::import("builtins");
    _py_compile = builtins.attr("compile");
    _py_exec = builtins.attr("exec");
    _py_eval = builtins.attr("eval");
    _py_next = builtins.attr("next");

    _py_zipfile_module = py::module_::import("zipfile");
    _py_zipfile_class = _py_zipfile_module.attr("ZipFile");
    _py_stop_iteration_type = py::eval("StopIteration");
}
