#pragma once
#include <memory>
#include <queue>
#include "globals.hpp"
#include <plog/Formatters/TxtFormatter.h>// Step1: include the headers
#include <plog/Appenders/ColorConsoleAppender.h>// Step1: include the headers
#include <plog/Initializers/RollingFileInitializer.h>


#include "lib_recomp.hpp"
#include "pyobject_slotmap.hpp"


class PyInterpreterController {
public:
    PyThreadState* py_main_thread = NULL;
    std::unordered_map<REPY_Handle, REPY_HandleEntry> py_objects_umap;
    PyObjectSlotMap py_objects_smap;
    std::queue<REPY_Handle> suh_release_queue;

    REPY_Handle next_handle_val = 1;

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
    py::function py_next;

    py::object py_stop_iteration_type;

    uint8_t* rdram;
    bool use_slotmap = false;

    PyInterpreterController(plog::Severity severity, fs::path mod_dir, bool p_use_slotmap);
    ~PyInterpreterController();

    // Handle Operations:
    REPY_Handle get_new_handle_value();
    REPY_Handle create_handle_and_steal(py::object* obj);
    REPY_Handle create_handle(py::object* obj);
    py::object* get_py_object(REPY_Handle handle);
    bool is_valid_handle(REPY_Handle handle);
    bool get_handle_suh(REPY_Handle handle);
    void set_handle_suh(REPY_Handle handle, bool is_single_use);
    void release_suh_handles();
    void release_handle(REPY_Handle handle);

    // Error Operations:
    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    REPY_Handle get_py_error_type_handle();
    REPY_Handle get_py_error_trace_handle();
    REPY_Handle get_py_error_value_handle();
    void clear_py_error();

    py::module_ construct_module(std::string module_name, std::string module_code, bool add_to_sys); 
    int random_in_range(int low, int high);

    void set_rdram(uint8_t* p_rdram);
};

extern std::shared_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(REPY_Handle, pos))