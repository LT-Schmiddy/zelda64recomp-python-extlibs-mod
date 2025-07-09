#pragma once

#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;
namespace fs = std::filesystem;

// Incbin stuff
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX 
#include <incbin.h>

#define INCLUDE_PYTHON_SRC_MODULE(module_name, module_file) \
INCTXT(module_name ## _code, module_file); \
PYBIND11_EMBEDDED_MODULE(module_name, m) { \
    py::exec((const char*)module_name ## _code_data, m.attr("__dict__")); \
} 

