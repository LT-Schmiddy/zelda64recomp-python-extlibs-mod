#pragma once
#include "globals.hpp"
#include "lib_recomp.hpp"

typedef int PyObjectHandle;
struct PyObjectHandleEntry {
    py::object py_object = py::none();
    bool is_single_use = false;
};

extern std::unordered_map<PyObjectHandle, PyObjectHandleEntry> py_objects;
int get_new_handle_value();
int create_py_handle(py::object obj);
py::object get_py_object(PyObjectHandle handle);

#define RECOMP_ARG_PYOBJECT(pos) get_py_object(RECOMP_ARG(PyObjectHandle, pos))