#include "global_root_controller.hpp"

GlobalRootController::GlobalRootController(uint8_t* rdram) {
    ZoneScoped;
    _rdram.store(rdram);
}

GlobalRootController::~GlobalRootController(){
    ZoneScoped;
}

// RDRAM:
uint8_t* GlobalRootController::get_rdram() {
    ZoneScoped;
    return _rdram.load();
}

void GlobalRootController::set_rdram(uint8_t* rdram) {
    ZoneScoped;
    _rdram.store(rdram);
}

// Controllers:

ThreadRootController* GlobalRootController::create_thread_root_controller(std::thread::id thread_id) {
    return new ThreadRootController(this, thread_id, _global_interps.size());
}

GlobalInterpreterController* GlobalRootController::get_global_interp_controller(REPY_InterpreterIndex index) {
    return _global_interps.at(index);
}