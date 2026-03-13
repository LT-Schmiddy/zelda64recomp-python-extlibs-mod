#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>
#include "globals.hpp"

#include "global_interpreter_controller.hpp"
#include "thread_root_controller.hpp"
#include "pyobject_slotmap.hpp"

// Responsible for tracking the location of recomp's rdram,
// the handle table, and the GlobalInterpreterControllers

// Globals can be run from any thread. Make sure everything is thread-safe.
class GlobalRootController {
public:
    // GlobalRootController() = delete;
    GlobalRootController(uint8_t* rdram);
    ~GlobalRootController();

    ThreadRootController* create_thread_root_controller(std::thread::id thread_id);
    GlobalInterpreterController* get_global_interp_controller(REPY_InterpreterIndex index);

    uint8_t* get_rdram();
    void set_rdram(uint8_t* rdram);


private:
    std::atomic<uint8_t*> _rdram;

    std::mutex _handles_mutex;
    PyObjectSlotMap _handles;

    std::mutex _global_interps_mutex;
    std::vector<GlobalInterpreterController*> _global_interps;
};