#include "recomp_mem.hpp"
#include "controller.hpp"
#include "lib_recomp.hpp"

PYBIND11_EMBEDDED_MODULE(_recomp_mem, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
    m.def("read_u8", [](int32_t ptr){
        uint8_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(uint8_t));
        return py::int_(val);
    });

    m.def("write_u8", [](int32_t ptr, py::int_ obj){
        uint8_t val = obj.cast<uint8_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(uint8_t));
    });
    
    m.def("read_u16", [](int32_t ptr){
        uint16_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(uint16_t));
        return py::int_(val);
    });

    m.def("write_u16", [](int32_t ptr, py::int_ obj){
        uint16_t val = obj.cast<uint16_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(uint16_t));
    });

    m.def("read_u32", [](int32_t ptr){
        uint32_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(uint32_t));
        return py::int_(val);
    });

    m.def("write_u32", [](int32_t ptr, py::int_ obj){
        uint32_t val = obj.cast<uint32_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(uint32_t));
    });

    m.def("read_u64", [](int32_t ptr){
        uint64_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(uint64_t));
        return py::int_(val);
    });

    m.def("write_u64", [](int32_t ptr, py::int_ obj){
        uint64_t val = obj.cast<uint64_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(uint64_t));
    });

    m.def("read_s8", [](int32_t ptr){
        int8_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(int8_t));
        return py::int_(val);
    });

    m.def("write_s8", [](int32_t ptr, py::int_ obj){
        int8_t val = obj.cast<int8_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(int8_t));
    });
    
    m.def("read_s16", [](int32_t ptr){
        int16_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(int16_t));
        return py::int_(val);
    });

    m.def("write_s16", [](int32_t ptr, py::int_ obj){
        int16_t val = obj.cast<int16_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(int16_t));
    });

    m.def("read_s32", [](int32_t ptr){
        int32_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(int32_t));
        return py::int_(val);
    });

    m.def("write_s32", [](int32_t ptr, py::int_ obj){
        int32_t val = obj.cast<int32_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(int32_t));
    });
    
    m.def("read_s64", [](int32_t ptr){
        int64_t val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(int64_t));
        return py::int_(val);
    });

    m.def("write_s64", [](int32_t ptr, py::int_ obj){
        int64_t val = obj.cast<int64_t>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(int64_t));
    });
    
    m.def("read_f32", [](int32_t ptr){
        float val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(float));
        return py::float_(val);
    });

    m.def("write_f32", [](int32_t ptr, py::float_ obj){
        float val = obj.cast<float>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(float));
    });
    
    m.def("read_f64", [](int32_t ptr){
        double val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(double));
        return py::float_(val);
    });

    m.def("write_f64", [](int32_t ptr, py::float_ obj){
        double val = obj.cast<double>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(double));
    });
    
    // Text
    m.def("read_char", [](int32_t ptr){
        char val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(char));
        return py::str(&val, 1);
    });

    m.def("write_char", [](int32_t ptr, py::str obj){
        char val = obj.cast<char>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(char));
    });

    m.def("read_byte_char", [](int32_t ptr){
        char val;
        memcpy_rev_from_recomp(controller->rdram, &val, ptr, sizeof(char));
        return py::bytes(&val, 1);
    });

    m.def("write_byte_char", [](int32_t ptr, py::bytes obj){
        char val = obj.cast<char>();
        memcpy_rev_to_recomp(controller->rdram, ptr, &val, sizeof(char));
    });

    m.def("read_str", [](int32_t ptr){
        std::u8string str = ptr_to_u8string(controller->rdram, ptr);
        return py::str(str);
    });

    m.def("read_str_n", [](int32_t ptr, uint32_t size){
        std::u8string str = ptr_to_u8string_n(controller->rdram, size, ptr);
        return py::str(str);
    });

    m.def("write_str_n", [](int32_t ptr, py::str str, uint32_t size){
        uint8_t* rdram = controller->rdram; // Used by MEM_B
        
        // write_size
        std::u8string cached_return_string = str.cast<std::u8string>();
        uint32_t str_len = cached_return_string.size() + 1;
        if (str_len > size) {
            str_len = size;
        }

        for (int i = 0; i < str_len; i++) {
            if (i == str_len - 1) {
                MEM_B(ptr, i) = '\0';
            } else {
                MEM_B(ptr, i) = cached_return_string.at(i);
            }
        }
    });

    m.def("read_byte_str", [](int32_t ptr){
        std::string str = ptr_to_string(controller->rdram, ptr);
        return py::bytes(str);
    });

    m.def("read_byte_str_n", [](int32_t ptr, uint32_t size){
        std::string str = ptr_to_string_n(controller->rdram, size, ptr);
        return py::bytes(str);
    });

    m.def("write_byte_str_n", [](int32_t ptr, py::bytes str, uint32_t size){
        uint8_t* rdram = controller->rdram; // Used by MEM_B
        
        // write_size
        std::string cached_return_string = str.cast<std::string>();
        uint32_t str_len = cached_return_string.size() + 1;
        if (str_len > size) {
            str_len = size;
        }

        for (int i = 0; i < str_len; i++) {
            if (i == str_len - 1) {
                MEM_B(ptr, i) = '\0';
            } else {
                MEM_B(ptr, i) = cached_return_string.at(i);
            }
        }
    });

    
    // Memory Blocks:
    m.def("read_bytes_n", [](int32_t ptr, uint32_t size) {
        uint8_t* rdram = controller->rdram;

        uint8_t* buf = new uint8_t[size];

        memcpy_from_recomp(rdram, buf, ptr, size);

        py::bytes retVal = py::bytes((char*)buf, size);
        delete[] buf;

        return retVal;
    });

    m.def("write_bytes_n", [](int32_t ptr, py::bytes bytes, uint32_t size) {
        uint8_t* rdram = controller->rdram; // Used by MEM_B

        uint32_t index = 0;
        for (auto byte : bytes) {
            if (index == size) {
                break;
            } else {
                index++;
            }
            
            MEM_B(ptr++, 0) = byte.cast<uint8_t>();
        }
    });

    m.def("read_bytearray_n", [](int32_t ptr, uint32_t size) {
        uint8_t* rdram = controller->rdram;

        uint8_t* buf = new uint8_t[size];

        memcpy_from_recomp(rdram, buf, ptr, size);

        py::bytearray retVal = py::bytearray((char*)buf, size);
        delete[] buf;

        return retVal;
    });

    m.def("write_bytearray_n", [](int32_t ptr, py::bytearray bytes, uint32_t size) {
        uint8_t* rdram = controller->rdram; // Used by MEM_B

        uint32_t index = 0;
        for (auto byte : bytes) {
            if (index == size) {
                break;
            } else {
                index++;
            }

            MEM_B(ptr++, 0) = byte.cast<uint8_t>();
        }
    });


}