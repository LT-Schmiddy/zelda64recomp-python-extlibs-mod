#pragma once

#include "globals.hpp"


namespace embedded_import {

py::module_ construct_module(std::string module_name, std::string module_code, bool add_to_sys); 
}