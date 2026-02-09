#include <fstream>
#include "embed_handler.hpp"

// Contains code used to handle the embedding of Python's standard library.

// This zip file is prepared by CMake during the configuration process.
// It contains all the .py files of the Python standard library.
INCBIN(python_stdlib, "python_stdlib.zip");

// Interestingly, MSVC has trouble using the incbin macro. And by default, clang
// Seems to have the same issue when running on windows. Not entirely sure why.
// However, zig is able to properly compile and include this in windows builds.
// For that reason, zig is critical for compiling this project on Windows,
// and stdlib extraction is disabled on non-zig Windows builds.
void extract_python_stdlib(fs::path output_file) {
    if (fs::exists(output_file)) {
        PLOGI.printf("Python's standard library already exists. No need for extraction.");
        return;
    }

#ifdef _MSC_VER
    PLOGE.printf("Extracting the Python standard library is not supported on this build.");
#else
    PLOGI.printf("Extracting the Python standard library to %s...", output_file.string().c_str());
    PLOGD.printf("std::size = %i\n", python_stdlib_size);

    std::ofstream outFile(output_file, std::ios::out | std::ios::binary);
    outFile.write((const char*)python_stdlib_data, python_stdlib_size);
    outFile.close();
#endif
}

// Windows builds of Python come with several additional native modules that require special handling
// and relocation. The packaging process renames the .pyd files to .dll so that recomp will extract them
// when on windows. Here, we'll reverse that process and move the files to a more suitable location.
#ifdef _WIN32
static const char* dll_modules[] = {
    "pyexpat.dll",
    "select.dll",
    "unicodedata.dll",
    "winsound.dll",
    "_asyncio.dll",
    "_bz2.dll",
    "_ctypes.dll",
    "_ctypes_test.dll",
    "_decimal.dll",
    "_elementtree.dll",
    "_hashlib.dll",
    "_lzma.dll",
    "_multiprocessing.dll",
    "_overlapped.dll",
    "_queue.dll",
    "_socket.dll",
    "_sqlite3.dll",
    "_ssl.dll",
    "_testbuffer.dll",
    "_testcapi.dll",
    "_testclinic.dll",
    "_testclinic_limited.dll",
    "_testconsole.dll",
    "_testimportmultiple.dll",
    "_testinternalcapi.dll",
    "_testlimitedcapi.dll",
    "_testmultiphase.dll",
    "_testsinglephase.dll",
    "_tkinter.dll",
    "_uuid.dll",
    "_wmi.dll",
    "_zoneinfo.dll",
    NULL
};

static const char* other_dlls[] = {
    "libcrypto-3-x64.dll",
    "libffi-8.dll",
    "libssl-3-x64.dll",
    "sqlite3.dll",
    "tcl86t.dll",
    "tk86t.dll",
    NULL
};


#define PY_NATIVE_EXTENSION ".pyd"
void setup_python_stdlib_dlls(fs::path mod_dir, fs::path dll_dir) {

    PLOGI.printf("Checking extensions on native modules...");
    if (!fs::exists(dll_dir)) {
        fs::create_directories(dll_dir);
    }

    // Handle PYDs
    for (int i = 0; dll_modules[i] != NULL; i++) {
        fs::path target_module = fs::path(mod_dir).append(dll_modules[i]);
        fs::path renamed_module = fs::path(dll_dir).append(target_module.filename().string().c_str()).replace_extension(PY_NATIVE_EXTENSION);

        if (!fs::exists(target_module) && fs::exists(renamed_module)) {
            PLOGD.printf("No rename required for native module '%s'.", renamed_module.string().c_str());
            continue;
        }

        else if (!fs::exists(target_module) && !fs::exists(renamed_module)) {
            PLOGD.printf("Renamed native module from '%s' missing.", target_module.string().c_str());
            continue;
        }

        else if (fs::exists(target_module) && fs::exists(renamed_module)) {
            fs::remove(renamed_module);
            PLOGD.printf("Deleted old renamed native module '%s'", renamed_module.string().c_str());
        }

        fs::rename(target_module, renamed_module);
        
        PLOGD.printf("Moved native module from '%s' to '%s'\n", target_module.string().c_str(), renamed_module.string().c_str());
    }

    // Handle DLLs
    for (int i = 0; other_dlls[i] != NULL; i++) {
        fs::path target_module = fs::path(mod_dir).append(other_dlls[i]);
        fs::path renamed_module = fs::path(dll_dir).append(target_module.filename().string().c_str());

        if (!fs::exists(target_module) && fs::exists(renamed_module)) {
            PLOGD.printf("No rename required for native dll '%s'.", renamed_module.string().c_str());
            continue;
        }

        else if (!fs::exists(target_module) && !fs::exists(renamed_module)) {
            PLOGD.printf("Renamed native dll from '%s' missing.", target_module.string().c_str());
            continue;
        }

        else if (fs::exists(target_module) && fs::exists(renamed_module)) {
            fs::remove(renamed_module);
            PLOGD.printf("Deleted old renamed native dll '%s'", renamed_module.string().c_str());
        }

        fs::rename(target_module, renamed_module);
        
        PLOGD.printf("Moved native dll from '%s' to '%s'\n", target_module.string().c_str(), renamed_module.string().c_str());
    }
}
#endif