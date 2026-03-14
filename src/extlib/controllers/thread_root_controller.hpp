#pragma once
#include <thread>
#include <stack>
#include "globals.hpp"

class ThreadRootController;

#include "global_root_controller.hpp"
#include "thread_interpreter_controller.hpp"

// Responsible for managing the thread's subinterpreter stack
// and the ThreadInterpreterControllers for that thread.

// This object will only be used by a single thread. No thread-safety code required here.
class ThreadRootController {
public:
    ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count);
    ~ThreadRootController();

    uint8_t* get_rdram();
    void set_rdram(uint8_t* rdram);

    // Interpreters:
    REPY_InterpreterIndex get_current_interp_index();
    ThreadInterpreterController* get_interp(REPY_InterpreterIndex index);
    ThreadInterpreterController* get_current_interp();
    void push_interp_index(REPY_InterpreterIndex handle);
    void pop_interp_index();

    // Handles:
    REPY_Handle create_handle(py::object* obj);
    REPY_Handle copy_handle(REPY_Handle handle);
    REPY_InterpreterIndex get_py_object_interpreter(REPY_Handle handle);
    py::object* get_py_object(REPY_Handle handle);
    bool is_valid_handle(REPY_Handle handle);
    bool get_handle_suh(REPY_Handle handle);
    void set_handle_suh(REPY_Handle handle, bool is_single_use);
    void release_suh_handles();
    void release_handle(REPY_Handle handle);

    // PyObject:
    py::function py_compile();
    py::function py_exec();
    py::function py_eval();
    py::function py_next();
    py::object py_stop_iteration_type();
    py::object get_zipfile_from_path(std::u8string filepath);

    // Errors:
    bool is_error_set();
    void handle_exception(py::error_already_set* e);
    py::object get_py_error_type();
    py::object get_py_error_trace();
    py::object get_py_error_value();
    void clear_py_error();
    
    py::module_ construct_module(std::u8string module_name, std::u8string module_code, bool add_to_sys); 

private:
    std::thread::id _thread_id;
    GlobalRootController* _global_root;
    std::stack<REPY_InterpreterIndex> _interp_index_stack;
    std::vector<ThreadInterpreterController*> _thread_interps;
};