#pragma once
#include <memory>
#include <queue>
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
    std::unordered_map<PyObjectHandle, PyObjectHandleEntry> py_objects;
    std::queue<PyObjectHandle> suh_release_queue;

    plog::RollingFileAppender<plog::TxtFormatter>* file_appender = NULL;
    plog::ColorConsoleAppender<plog::TxtFormatter>* console_appender = NULL;
    plog::Logger<0>* log = NULL;

    bool is_py_error_set = false;
    py::object last_error_type = py::none();
    py::object last_error_trace = py::none();
    py::object last_error_value = py::none();

    py::function py_compile;
    py::function py_exec;
    py::function py_eval;

    uint8_t* rdram;

    PyInterpreterController(plog::Severity severity, fs::path mod_dir);
    ~PyInterpreterController();

    // Handle Operations:
    PyObjectHandle get_new_handle_value();
    PyObjectHandle create_handle_and_steal(py::object* obj);
    PyObjectHandle create_handle(py::object* obj);
    py::object* get_py_object(PyObjectHandle handle);
    bool get_handle_suh(PyObjectHandle handle);
    void set_handle_suh(PyObjectHandle handle, bool is_single_use);
    void release_suh_handles();
    void release_handle(PyObjectHandle handle);

    // Error Operations:
    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    PyObjectHandle get_py_error_type_handle();
    PyObjectHandle get_py_error_trace_handle();
    PyObjectHandle get_py_error_value_handle();
    void clear_py_error();

    py::module_ construct_module(std::string module_name, std::string module_code, bool add_to_sys); 
    int random_in_range(int low, int high);

    void set_rdram(uint8_t* p_rdram);

};

extern std::shared_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(PyObjectHandle, pos))