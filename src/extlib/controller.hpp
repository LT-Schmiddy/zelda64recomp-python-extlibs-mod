#pragma once
#include <memory>
#include <queue>
#include <stack>
#include <vector>

#include "globals.hpp"
#include <plog/Formatters/TxtFormatter.h>// Step1: include the headers
#include <plog/Appenders/ColorConsoleAppender.h>// Step1: include the headers
#include <plog/Initializers/RollingFileInitializer.h>

#include "lib_recomp.hpp"
#include "pyobject_slotmap.hpp"
#include "subinterpreter.hpp"

class PyInterpreterController {
public:
    PyInterpreterController(plog::Severity severity, bool log_to_file, fs::path mod_dir, std::queue<fs::path>* registered_nrms);
    ~PyInterpreterController();

    uint32_t create_subinterpreter();

    // Handle Operations:
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

    REPY_Handle get_zipfile_from_path(std::u8string filepath);

    py::module_ construct_module(std::string module_name, std::string module_code, bool add_to_sys); 

    uint8_t* get_rdram();
    void set_rdram(uint8_t* p_rdram);

private:
    PyThreadState* py_main_thread = NULL;
    PyObjectSlotMap py_objects_smap;
    std::queue<REPY_Handle> suh_release_queue;
    std::stack<REPY_SubinterpHandle> subinterp_handle_stack;
    std::vector<PySubinterpreterController*> subinterpreters;

    plog::RollingFileAppender<plog::TxtFormatter>* file_appender = NULL;
    plog::ColorConsoleAppender<plog::TxtFormatter>* console_appender = NULL;
    plog::Logger<0>* log = NULL;
    uint8_t* rdram;

};

extern std::unique_ptr<PyInterpreterController> controller;

#define RECOMP_ARG_PYOBJECT(pos) controller->get_py_object(RECOMP_ARG(REPY_Handle, pos))