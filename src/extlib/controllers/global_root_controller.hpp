#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>
#include "globals.hpp"

class GlobalRootController;

#include "global_interpreter_controller.hpp"
#include "thread_root_controller.hpp"
#include "pyobject_slotmap.hpp"

// Responsible for tracking the location of recomp's rdram,
// the handle table, and the GlobalInterpreterControllers

// Globals can be run from any thread. Make sure everything is thread-safe.
class GlobalRootController {
public:
    // GlobalRootController() = delete;
    GlobalRootController(uint8_t* rdram, REPY_InterpreterIndex subinterp_count);
    ~GlobalRootController();

    uint8_t* get_rdram();
    void set_rdram(uint8_t* rdram);

    // Threading:
    ThreadRootController* create_thread_root_controller(std::thread::id thread_id);

    // Interpreters:
    GlobalInterpreterController* get_global_interp_controller(REPY_InterpreterIndex index);

    // Handles:
    REPY_Handle create_handle(py::object* obj, REPY_InterpreterIndex interp_index);
    REPY_Handle copy_handle(REPY_Handle handle);
    REPY_InterpreterIndex get_py_object_interpreter(REPY_Handle handle);
    py::object* get_py_object(REPY_Handle handle, REPY_InterpreterIndex current_interp_index);
    bool is_valid_handle(REPY_Handle handle);
    bool get_handle_suh(REPY_Handle handle);
    void set_handle_suh(REPY_Handle handle, bool is_single_use);
    void release_suh_handles();
    void release_handle(REPY_Handle handle);

private:
    std::atomic<uint8_t*> _rdram;

    std::mutex _handles_mutex;
    std::queue<REPY_Handle> _suh_release_queue;
    PyObjectSlotMap _handles;

    std::mutex _global_interps_mutex;
    std::vector<GlobalInterpreterController*> _global_interps;
};