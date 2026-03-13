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
    GlobalRootController* _global_root = nullptr;
    GlobalInterpreterController* _global_interp = nullptr;
};