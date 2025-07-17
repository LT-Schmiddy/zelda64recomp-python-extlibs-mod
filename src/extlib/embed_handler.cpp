#include <fstream>
#include "embed_handler.hpp"

INCBIN(python_stdlib, "python313_stdlib.zip");

void extract_python_stdlib(fs::path output_file) {
    if (fs::exists(output_file)) {
        PLOGI.printf("Python's standard library already exists. No need for extraction.");
        return;
    }

#ifdef _MSC_VER
    PLOGE.printf("Extracting the Python standard library is not supported on this build.");
#else
    PLOGI.printf("Extracting the Python standard library...");
    PLOGD.printf("std::size = %i\n", python_stdlib_size);

    std::ofstream outFile(output_file, std::ios::out | std::ios::binary);
    outFile.write((const char*)python_stdlib_data, python_stdlib_size);
    outFile.close();
#endif
}

#ifdef _WIN32
const char* dll_modules[] = {
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

#define PY_NATIVE_EXTENSION ".pyd"

#endif
void setup_python_stdlib_dlls(fs::path dll_dir) {
#ifdef _WIN32
    PLOGI.printf("Checking extensions on native modules...");
    for (int i = 0; dll_modules[i] != NULL; i++) {
        fs::path target_module = fs::path(dll_dir).append(dll_modules[i]);
        fs::path renamed_module = fs::path(target_module).replace_extension(PY_NATIVE_EXTENSION);

        if (fs::exists(renamed_module)) {
            PLOGD.printf("No rename required for native module '%s'.", renamed_module.string().c_str());
            continue;
        }

        if (!fs::exists(target_module)) {
            PLOGD.printf("Renamed native module from '%s' missing.", target_module.string().c_str());
            continue;
        }

        fs::rename(target_module, renamed_module);
        PLOGD.printf("Renamed native module from '%s' to '%s'\n", target_module.string().c_str(), renamed_module.string().c_str());
    }
#endif
}