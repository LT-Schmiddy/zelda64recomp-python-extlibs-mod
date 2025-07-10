#include "utils.hpp"

py::object py_compile;
py::object py_exec;
py::object py_eval;

int random_in_range(int low, int high) {
    return rand() % (high - low + 1) + low;
}

void collect_py_functions() {
    py::gil_scoped_acquire gil;
    auto builtins = py::module_::import("builtins");

    py_compile = builtins.attr("compile");
    py_exec = builtins.attr("exec");
    py_eval = builtins.attr("eval");
}