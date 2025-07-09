
#include "embedded_import.hpp"

namespace embedded_import {

py::module_ construct_module(std::string module_name, std::string module_code) {
    auto types = py::module_::import("types");
    auto sys = py::module_::import("sys");
    
    auto new_mod = types.attr("ModuleType")(module_name);

    try {
        py::exec(module_code, new_mod.attr("__dict__"));
    } catch (py::error_already_set &e) {
        std::cout << e.what();
        return py::none();
    }

    py::dict sys_modules = sys.attr("modules");
    sys_modules[module_name.c_str()] = new_mod;

    return new_mod;
};

}