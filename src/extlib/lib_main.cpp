#include <iostream>
#include <filesystem>
#include <stdio.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "lib_recomp.hpp"

namespace py = pybind11;
namespace fs = std::filesystem;

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

PYBIND11_EMBEDDED_MODULE(nrm_importer, m) {
    // `m` is a `py::module_` which is used to bind functions and classes
    m.def("add", [](int i, int j) {
        return i + j;
    });
}

RECOMP_DLL_FUNC(PythonNative_Init) {
    std::u8string mod_dir_text = RECOMP_ARG_U8STR(0);
    fs::path mod_dir(mod_dir_text);
    fs::path mod_dir_DLLs = fs::path(mod_dir).append("DLLs");
    fs::path mod_dir_Lib = fs::path(mod_dir).append("Lib");

    printf("Mod Folder: %s\n", (char*)mod_dir_text.c_str());

    // start the interpreter and keep it alive
    py::initialize_interpreter(); 
    // Setting the module search path for the interpreter
    auto sys = py::module_::import("sys");
    auto sys_path = sys.attr("path");
    sys_path = sys.attr("path");
    sys_path.attr("clear")();
    sys_path.attr("append")(mod_dir.string());
    sys_path.attr("append")(mod_dir_DLLs.string());
    sys_path.attr("append")(mod_dir_Lib.string());

    py::print(sys_path);

    RECOMP_RETURN(int, 0);
}

RECOMP_DLL_FUNC(PythonNative_Call) {
    
    RECOMP_RETURN(int, 0);
}
