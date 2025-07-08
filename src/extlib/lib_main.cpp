#include <iostream>
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "lib_recomp.hpp"

namespace py = pybind11;

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

RECOMP_DLL_FUNC(PythonNative_Init) {
    std::string mod_text = RECOMP_ARG_STR(0);
    std::cout << mod_text << "\n";


    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    py::print("Hello, World!"); // use the Python API

    RECOMP_RETURN(int, 0);
}
