#pragma once
#include <mutex>
#include <vector>
#include "globals.hpp"

#include "global_interpreter_controller.hpp"
#include "pyobject_slotmap.hpp"

// Responsible for tracking the location of recomp's rdram,
// the handle table, and the GlobalInterpreterControllers
class GlobalRootController {
public:
    // GlobalRootController() = delete;
    GlobalRootController();
    ~GlobalRootController();
private:
    uint8_t* _rdram;
    PyObjectSlotMap _py_objects_smap;
    std::vector<GlobalInterpreterController> _interpreters;
};