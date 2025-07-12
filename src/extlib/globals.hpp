#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>


#ifdef _DEBUG
#undef _DEBUG
// Need to turn this off for debug builds, otherwise we get an error on shutdown debug builds.
#define PYBIND11_NO_ASSERT_GIL_HELD_INCREF_DECREF
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#define _DEBUG
#else 
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#endif

namespace py = pybind11;
namespace fs = std::filesystem;

// Incbin stuff
#define INCBIN_SILENCE_BITCODE_WARNING
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX 
#include <incbin.h>

// #define INCLUDE_PYTHON_SRC_MODULE(module_name, module_file) \
// INCTXT(module_name ## _code, module_file); \
// PYBIND11_EMBEDDED_MODULE(module_name, m) { \
//     py::gil_scoped_acquire gil; \
//     auto builtins = py::module_::import("builtins"); \
//     auto py_compile = builtins.attr("compile"); \
//     auto py_exec = builtins.attr("exec"); \
//     auto p_code = py_compile((const char*)module_name ## _code_data, module_file, "exec"); \
//     py_exec(p_code, m.attr("__dict__")); \
// } 

