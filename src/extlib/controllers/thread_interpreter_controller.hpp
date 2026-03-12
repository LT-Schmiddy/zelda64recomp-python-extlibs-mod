#pragma once
#include "globals.hpp"
#include "global_interpreter_controller.hpp"
#include "global_root_controller.hpp"


// Manages the activation and deactivation of this interpreter on this thread:
class ThreadInterpreterController {
public:
    ThreadInterpreterController();
    ~ThreadInterpreterController();
private:
    GlobalRootController* _global_root = NULL;
    GlobalInterpreterController* _global_interp = NULL;
    ThreadRootController* _thread_root = NULL;
};