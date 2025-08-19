#pragma once

#include "globals.hpp"

void extract_python_stdlib(fs::path mod_dir);
void setup_python_stdlib_dlls(fs::path mod_dir, fs::path dll_dir);