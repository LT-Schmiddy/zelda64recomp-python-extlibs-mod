#include "globals.hpp"
#include "lib_recomp.hpp"

py::bytes recomp_memcpy_to_py_bytes(uint8_t* rdram, int32_t ptr, uint32_t size, bool reverse);
py::bytearray recomp_memcpy_to_py_bytearray(uint8_t* rdram, int32_t ptr, uint32_t size, bool reverse);