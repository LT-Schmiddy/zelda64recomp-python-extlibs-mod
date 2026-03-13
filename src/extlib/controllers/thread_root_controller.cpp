#include "thread_root_controller.hpp"

ThreadRootController::ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count) {
    _global_root = global_root;
    _thread_id = thread_id;
    _thread_interps.resize(interp_count, nullptr);
}

ThreadRootController::~ThreadRootController() {

}