#include "thread_root_controller.hpp"

ThreadRootController::ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count) {
    ZoneScoped;
    _global_root = global_root;
    _thread_id = thread_id;
    _thread_interps.resize(interp_count, nullptr);

    PLOGI.printf("Creating ThreadRootController for thread %llu. Subinterpreter count: %u", thread_id, _thread_interps.size());
}

ThreadRootController::~ThreadRootController() {
    ZoneScoped;

}

uint8_t* ThreadRootController::get_rdram() {
    return _global_root->get_rdram();
}

void ThreadRootController::set_rdram(uint8_t* rdram) {
    _global_root->set_rdram(rdram);
}

REPY_InterpreterIndex ThreadRootController::get_current_interp_index() {
    ZoneScoped;
    if (_interp_index_stack.empty()) {
        return -1;
    }
    return _interp_index_stack.top();
}

ThreadInterpreterController* ThreadRootController::get_interp(REPY_InterpreterIndex index) {
    ZoneScoped;
    REPY_InterpreterIndex upper = _thread_interps.size();
    if (index < 0 || index > upper) {
        PLOGF.printf("REPY_InterpreterIndex %i is out of range.", index);
    }
    assert(index >= 0 && index < _thread_interps.size());

    ThreadInterpreterController* retVal = _thread_interps.at(index);
    if (retVal == nullptr) {
        retVal = new ThreadInterpreterController(_global_root, _global_root->get_global_interp_controller(index));
        _thread_interps.at(index) = retVal;
    }
    return retVal;
}

ThreadInterpreterController* ThreadRootController::get_current_interp() {
    ZoneScoped;
    if (_interp_index_stack.empty()) {
        PLOGF.printf("No interpreter selected. Make sure you are using REPY_PushInterpreter and REPY_PopInterpreter correctly");
    }
    assert(!_interp_index_stack.empty());

    return get_interp(get_current_interp_index());
}

void ThreadRootController::push_interp_index(REPY_InterpreterIndex index) {
    ZoneScoped;
    if (_interp_index_stack.empty()) {
        _interp_index_stack.push(index);
        get_interp(index)->activate();
    } else {
        REPY_InterpreterIndex old_index = _interp_index_stack.top();
        _interp_index_stack.push(index);
        if (old_index != index) {
            get_interp(old_index)->deactivate();
            get_interp(index)->activate();
        }
    }
}

void ThreadRootController::pop_interp_index() {
    ZoneScoped;
    REPY_InterpreterIndex old_index = _interp_index_stack.top();
    _interp_index_stack.pop();
    if (_interp_index_stack.empty()) {
        _thread_interps.at(old_index)->deactivate();
    } else {
        REPY_InterpreterIndex new_index = _interp_index_stack.top();
        if (old_index != new_index) {
            _thread_interps.at(old_index)->deactivate();
            _thread_interps.at(new_index)->activate();
        }
    }
}

// Handles -  Wrapping the global root controller:
REPY_Handle ThreadRootController::create_handle(py::object* obj) {
    ZoneScoped;
    return _global_root->create_handle(obj, get_current_interp_index());
}

REPY_InterpreterIndex ThreadRootController::get_py_object_interpreter(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->get_py_object_interpreter(handle);
}

py::object* ThreadRootController::get_py_object(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->get_py_object(handle, get_current_interp_index());
}

bool ThreadRootController::is_valid_handle(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->is_valid_handle(handle);
}

bool ThreadRootController::get_handle_suh(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->get_handle_suh(handle);
}
void ThreadRootController::set_handle_suh(REPY_Handle handle, bool is_single_use) {
    ZoneScoped;
    _global_root->set_handle_suh(handle, is_single_use);
}
void ThreadRootController::release_suh_handles() {
    ZoneScoped;
    _global_root->release_suh_handles();
}

void ThreadRootController::release_handle(REPY_Handle handle) {
    ZoneScoped;
    _global_root->release_handle(handle);
}

// Interpreter access: wrapping the thread interpreter controller:
// PyObject:
py::function ThreadRootController::py_compile() {
    return get_current_interp()->py_compile();
}
py::function ThreadRootController::py_exec() {
    return get_current_interp()->py_exec();
}

py::function ThreadRootController::py_eval() {
    return get_current_interp()->py_eval();
}

py::function ThreadRootController::py_next() {
    return get_current_interp()->py_next();
}

py::object ThreadRootController::py_stop_iteration_type() {
    return get_current_interp()->py_stop_iteration_type();
}

py::object ThreadRootController::get_zipfile_from_path(std::u8string filepath) {
    return get_current_interp()->get_zipfile_from_path(filepath);
}

// Errors:
bool ThreadRootController::is_error_set() {
    return get_current_interp()->is_error_set();
}

void ThreadRootController::handle_exception(py::error_already_set* e) {
    return get_current_interp()->handle_exception(e);
}

py::object ThreadRootController::get_py_error_type() {
    return get_current_interp()->get_py_error_type();
}

py::object ThreadRootController::get_py_error_trace() {
    return get_current_interp()->get_py_error_trace();
}

py::object ThreadRootController::get_py_error_value() {
    return get_current_interp()->get_py_error_value();
}

void ThreadRootController::clear_py_error() {
    get_current_interp()->clear_py_error();
}

py::module_ ThreadRootController::construct_module(std::u8string module_name, std::u8string module_code, bool add_to_sys) {
    ZoneScoped;
    py::gil_scoped_acquire gil;
    
    auto types = py::module_::import("types");
    auto new_mod = types.attr("ModuleType")(module_name);

    try {
        py::exec((const char*)module_code.c_str(), new_mod.attr("__dict__"));
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        return py::none();
    }

    if (add_to_sys) {
        auto sys = py::module_::import("sys");
        py::dict sys_modules = sys.attr("modules");
        sys_modules[(const char*)module_name.c_str()] = new_mod;
    }

    return new_mod;
};