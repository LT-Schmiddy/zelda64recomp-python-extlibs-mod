#include "recomp_interact.hpp"
#include "controller.hpp"
#include "lib_recomp.hpp"

PYBIND11_EMBEDDED_MODULE(recomp_mem, m) {

    m.def("write_bytes_n", [](int32_t ptr, py::bytes bytes) {
        uint8_t* rdram = controller->rdram;

        for (auto byte : bytes) {
            MEM_B(ptr++, 0) = byte.cast<uint8_t>();
        }
    });

    m.def("write_bytes_n", [](int32_t ptr, py::bytearray bytes) {
        uint8_t* rdram = controller->rdram;

        for (auto byte : bytes) {
            MEM_B(ptr++, 0) = byte.cast<uint8_t>();
        }
    });

    m.def("read_bytes_n", [](int32_t ptr, int size) {
        uint8_t* rdram = controller->rdram;

        uint8_t* buf = new uint8_t[size];

        memcpy_from_recomp(rdram, buf, ptr, size);

        py::bytes retVal = py::bytes((char*)buf);
        delete[] buf;

        return retVal;
    });
}