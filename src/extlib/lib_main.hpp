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
    std::u8string cached_return_u8string;
    std::string cached_return_string;

    PyInterpreterController();

    ~PyInterpreterController();

    PyObjectHandle get_new_handle_value();

    int create_py_handle(py::object obj);

    py::object get_py_object(PyObjectHandle handle);

};

extern std::shared_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(PyObjectHandle, pos))