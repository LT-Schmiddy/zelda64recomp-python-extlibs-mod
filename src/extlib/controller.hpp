#pragma once
#include <memory>
#include <plog/Log.h>
#include <plog/Formatters/TxtFormatter.h>// Step1: include the headers
#include <plog/Appenders/ColorConsoleAppender.h>// Step1: include the headers
#include <plog/Initializers/RollingFileInitializer.h>

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

    plog::RollingFileAppender<plog::TxtFormatter>* file_appender = NULL;
    plog::ColorConsoleAppender<plog::TxtFormatter>* console_appender = NULL;
    plog::Logger<0>* log = NULL;

    py::object py_compile;
    py::object py_exec;
    py::object py_eval;

    PyInterpreterController(plog::Severity severity, fs::path mod_dir);
    ~PyInterpreterController();

    // Handle Operations:
    PyObjectHandle get_new_handle_value();
    int create_handle(py::object obj);
    py::object get_py_object(PyObjectHandle handle);
    bool get_handle_suh(PyObjectHandle handle);
    void set_handle_suh(PyObjectHandle handle, bool is_single_use);
    void release_handle(PyObjectHandle handle);

    py::module_ construct_module(std::string module_name, std::string module_code, bool add_to_sys); 
    int random_in_range(int low, int high);

};

extern std::shared_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(PyObjectHandle, pos))