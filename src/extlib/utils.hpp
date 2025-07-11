#pragma once
#include "globals.hpp"

extern py::object py_compile;
extern py::object py_exec;
extern py::object py_eval;

int random_in_range(int low, int high);
void collect_py_functions();