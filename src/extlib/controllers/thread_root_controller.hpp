#pragma once
#include <thread>
#include <stack>
#include "globals.hpp"

#include "global_root_controller.hpp"
#include "thread_interpreter_controller.hpp"

// Responsible for managing the thread's subinterpreter stack
// and the ThreadInterpreterControllers for that thread.
class ThreadRootController {
public:
    ThreadRootController(std::thread::id);
    ~ThreadRootController();
private:
    GlobalRootController* _global_root;

    std::stack<REPY_InterpreterIndex> _interp_index_stack;
    std::vector<ThreadInterpreterController> _interpreters;
};