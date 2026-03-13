#include "global_root_controller.hpp"

GlobalRootController::GlobalRootController(uint8_t* rdram) {
    ZoneScoped;
    _rdram.store(rdram);

    // Adding main interpreter
    GlobalInterpreterController* _interp = new GlobalInterpreterController(0);
    _global_interps.push_back(_interp);
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
    ZoneScoped;
    std::lock_guard lock(_global_interps_mutex);
    uint32_t interp_size = _global_interps.size();
    return new ThreadRootController(this, thread_id, interp_size);
}

// Interpreters
REPY_InterpreterIndex GlobalRootController::create_global_interp_controller() {
    ZoneScoped;
    std::lock_guard lock(_global_interps_mutex);
    REPY_InterpreterIndex retVal = _global_interps.size();
    GlobalInterpreterController* _interp = new GlobalInterpreterController(retVal);
    _global_interps.push_back(_interp);

    PLOGI.printf("Created new subinterpreter with a handle of %u", retVal);

    return retVal;

}

GlobalInterpreterController* GlobalRootController::get_global_interp_controller(REPY_InterpreterIndex index) {
    ZoneScoped;
    std::lock_guard lock(_global_interps_mutex);
    return _global_interps.at(index);
}

// Handles:
REPY_Handle GlobalRootController::create_handle(py::object* obj, REPY_InterpreterIndex interp_index) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    REPY_Handle new_handle = _handles.add(obj, interp_index);

    PLOGD.printf("-> REPY_Handle 0x%08X created on interpreter %u", new_handle, interp_index);
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(*obj).cast<std::u8string>();
        PLOGV.printf("-> Handle %08X: %s", new_handle, repr_str.c_str());
    }
    return new_handle;
}

REPY_InterpreterIndex GlobalRootController::get_py_object_interpreter(REPY_Handle handle) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = _handles.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    return entry->interp_index;
}


py::object* GlobalRootController::get_py_object(REPY_Handle handle, REPY_InterpreterIndex current_interp_index) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = _handles.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    if (entry->interp_index != current_interp_index) {
        PLOGW.printf("REPY_Handle 0x%08X: accessing an interpreter %u object while interpreter %u is active", handle, entry->interp_index, current_interp_index);
    } 
    
    if (entry->is_single_use) {
        _suh_release_queue.push(handle);
        PLOGD.printf("-> REPY_Handle 0x%08X from interpreter %u accessed (SUH)", handle, entry->interp_index);
    } else {
        PLOGD.printf("-> REPY_Handle 0x%08X from interpreter %u accessed", handle, entry->interp_index);
    }
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(entry->py_object).cast<std::u8string>();
        PLOGV.printf("Handle 0x%08X: %s", handle, repr_str.c_str());
    }
    return &entry->py_object;
    
}

bool GlobalRootController::is_valid_handle(REPY_Handle handle) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    return _handles.has(handle);
}

bool GlobalRootController::get_handle_suh(REPY_Handle handle) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = _handles.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    return entry->is_single_use;
}

void GlobalRootController::set_handle_suh(REPY_Handle handle, bool is_single_use) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    if (handle == 0) {
        PLOGF.printf("REPY_Handle 0 was used in a case where a valid Python handle is required");
    } 
    assert(handle != 0);

    REPY_HandleEntry* entry = _handles.get(handle);
    if (entry == NULL) {
        PLOGF.printf("0x%08X is not a valid REPY_Handle");
    } 
    assert(entry != NULL);

    entry->is_single_use = is_single_use;
    PLOGD.printf("-> REPY_Handle 0x%08X setting SUH = %i", handle, is_single_use);
}

void GlobalRootController::release_suh_handles() {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    while (_suh_release_queue.size() > 0) {
        REPY_Handle handle = _suh_release_queue.front();
        PLOGD.printf("-> REPY_Handle 0x%08X released (SUH)", handle);
        IF_PLOG(plog::verbose) {
            std::u8string repr_str = py::repr(_handles.get(handle)->py_object).cast<std::u8string>();
            PLOGV.printf("-> Handle 0x%08X: %s", handle, repr_str.c_str());
        }
        _suh_release_queue.pop();
        _handles.del(handle);
    }
}

void GlobalRootController::release_handle(REPY_Handle handle) {
    ZoneScoped;
    std::lock_guard lock(_handles_mutex);
    PLOGD.printf("-> REPY_Handle 0x%08X released", handle);
    IF_PLOG(plog::verbose) {
        std::u8string repr_str = py::repr(_handles.get(handle)->py_object).cast<std::u8string>();
        PLOGV.printf("-> Handle 0x%08X: %s", handle, repr_str.c_str());

    }
    _handles.del(handle);
}