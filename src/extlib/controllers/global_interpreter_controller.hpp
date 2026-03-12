#pragma once
#include "globals.hpp"

// Holds references to interpreter-specific Python objects needed for the API.
class GlobalInterpreterController {
public:
    GlobalInterpreterController() = delete;
    GlobalInterpreterController(REPY_InterpreterIndex index);
    ~GlobalInterpreterController();

private:
    REPY_InterpreterIndex _index = 0;
};