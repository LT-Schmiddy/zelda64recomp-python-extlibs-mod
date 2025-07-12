#pragma once
#include <memory>

#include "globals.hpp"
#include "lib_recomp.hpp"

typedef int PyObjectHandle;
struct PyObjectHandleEntry {
    py::object py_object = py::none();
    bool is_single_use = false;
};



class PyInterpreterController {
public:
    PyThreadState* py_main_thread = NULL;
    std::unordered_map<int, PyObjectHandleEntry> py_objects;

    PyInterpreterController();
    ~PyInterpreterController();

    // Handle Operations:
    PyObjectHandle get_new_handle_value();
    int create_handle(py::object obj);
    py::object get_py_object(PyObjectHandle handle);
    bool get_handle_suh(PyObjectHandle handle);
    void set_handle_suh(PyObjectHandle handle, bool is_single_use);
    void release_handle(PyObjectHandle handle);

};

extern std::shared_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(PyObjectHandle, pos))