#include "globals.hpp"
#include "lib_recomp.hpp"

#include "embedded_import.hpp"
#include "incbin.h"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

INCLUDE_PYTHON_SRC_MODULE(incbin_module, "incbin_module.py");

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
    sys_path.attr("clear")();
    sys_path.attr("append")(mod_dir.string());
    sys_path.attr("append")(mod_dir_DLLs.string());
    sys_path.attr("append")(mod_dir_Lib.string());

    embedded_import::construct_module("test_module", R"(message = "Hello Alex"
)");

    try {
        py::exec(R"(
            import test_module, incbin_module
            print(test_module.message)
            print(incbin_module.message)
        )");
    } catch (py::error_already_set &e) {
        std::cout << e.what();
    }

    // py::print(sys_path);

    RECOMP_RETURN(int, 0);
}

RECOMP_DLL_FUNC(PythonNative_Call) {
    
    RECOMP_RETURN(int, 0);
}
