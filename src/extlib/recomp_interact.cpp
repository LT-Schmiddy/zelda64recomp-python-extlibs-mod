#include "recomp_interact.hpp"
#include "controller.hpp"
#include "lib_recomp.hpp"

PYBIND11_EMBEDDED_MODULE(recomp_mem, m) {

    m.def("write_bytes", [](int ptr, py::bytes bytes) {
        uint8_t* rdram = controller->rdram;

        for (auto byte : bytes) {
            MEM_B(ptr++, 0) = byte.cast<uint8_t>();
        }
    });

    m.def("read_bytes_n", [](int ptr, int size) {
        uint8_t* rdram = controller->rdram;

        uint8_t* buf = new uint8_t[size];

        for (int i = 0; i < size; i++) {
            buf[i] = MEM_B(ptr, i);
        }

        py::bytes retVal = py::bytes((char*)buf);
        delete[] buf;

        return retVal;
    });
}