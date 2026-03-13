#pragma once
#include <thread>
#include <stack>
#include "globals.hpp"

#include "global_root_controller.hpp"
#include "thread_interpreter_controller.hpp"

// Responsible for managing the thread's subinterpreter stack
// and the ThreadInterpreterControllers for that thread.

// This object will only be used by a single thread. No thread-safety code required here.
class ThreadRootController {
public:
    ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count);
    ~ThreadRootController();
private:
    std::thread::id _thread_id;
    GlobalRootController* _global_root;
    std::stack<REPY_InterpreterIndex> _interp_index_stack;
    std::vector<ThreadInterpreterController*> _thread_interps;
};