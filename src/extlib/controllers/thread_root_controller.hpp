#pragma once
#include <thread>
#include <stack>
#include "globals.hpp"

#include "global_root_controller.hpp"
#include "thread_interpreter_controller.hpp"

// Responsible for managing the thread's subinterpreter stack
// and the ThreadInterpreterControllers for that thread.

// This object will only be used by a single thread. No thread-safety code required here.
class ThreadRootController {
public:
    ThreadRootController(GlobalRootController* global_root, std::thread::id thread_id, uint32_t interp_count);
    ~ThreadRootController();

    REPY_Handle create_handle(py::object* obj);
    REPY_InterpreterIndex get_py_object_interpreter(REPY_Handle handle);
    py::object* get_py_object(REPY_Handle handle);
    bool is_valid_handle(REPY_Handle handle);
    bool get_handle_suh(REPY_Handle handle);
    void set_handle_suh(REPY_Handle handle, bool is_single_use);
    void release_suh_handles();
    void release_handle(REPY_Handle handle);

private:
    std::thread::id _thread_id;
    GlobalRootController* _global_root;
    std::stack<REPY_InterpreterIndex> _interp_index_stack;
    std::vector<ThreadInterpreterController*> _thread_interps;
};