#include "thread_root_controller.hpp"

ThreadRootController::ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count) {
    ZoneScoped;
    _global_root = global_root;
    _thread_id = thread_id;
    _thread_interps.resize(interp_count, nullptr);
}

ThreadRootController::~ThreadRootController() {
    ZoneScoped;

}

// Handles:
REPY_Handle ThreadRootController::create_handle(py::object* obj) {
    ZoneScoped;
    return _global_root->create_handle(obj, _interp_index_stack.top());
}

REPY_InterpreterIndex ThreadRootController::get_py_object_interpreter(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->get_py_object_interpreter(handle);
}

py::object* ThreadRootController::get_py_object(REPY_Handle handle) {
    ZoneScoped;
    return _global_root->get_py_object(handle, _interp_index_stack.top());
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
