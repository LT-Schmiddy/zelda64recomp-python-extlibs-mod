#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>

#include <tracy/Tracy.hpp>

// This is what allows us to use free-threaded Python
#define Py_GIL_DISABLED 1
#ifdef _DEBUG
#undef _DEBUG
// Need to turn this off for debug builds, otherwise we get an error on shutdown debug builds.
#define PYBIND11_NO_ASSERT_GIL_HELD_INCREF_DECREF
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/subinterpreter.h>
#define _DEBUG
#else 
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/subinterpreter.h>
#endif
#include <plog/Log.h>

namespace py = pybind11;
namespace fs = std::filesystem;

// Incbin stuff
#define INCBIN_SILENCE_BITCODE_WARNING
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX 
#include <incbin.h>

#define PYTHON_VERSION_STR "python314t"
#define PYTHON_PROGRAM_NAME "RecompExternalPython"

#define PYTHON_MAIN_INTERPRETER_HANDLE 0
typedef signed int REPY_InterpreterIndex;
typedef unsigned int REPY_Handle;

