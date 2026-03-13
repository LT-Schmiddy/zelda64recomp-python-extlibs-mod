#include "recomp_mem.hpp"
#include "lib_main.hpp"
#include "lib_recomp.hpp"

#include <algorithm>

// Internal module for recomp memory with automatic byteswapping.
PYBIND11_EMBEDDED_MODULE(_recomp_mem_managed, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
    ZoneScoped;

    m.def("read_u8", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_u8");
        l_controller->thread_check_exception();
        uint8_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(uint8_t));
        return py::int_(val);
    });

    m.def("write_u8", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_u8");
        l_controller->thread_check_exception();
        uint8_t val = obj.cast<uint8_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(uint8_t));
    });
    
    m.def("read_u16", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_u16");
        l_controller->thread_check_exception();
        uint16_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(uint16_t));
        return py::int_(val);
    });

    m.def("write_u16", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_u16");
        l_controller->thread_check_exception();
        uint16_t val = obj.cast<uint16_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(uint16_t));
    });

    m.def("read_u32", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_u32");
        l_controller->thread_check_exception();
        uint32_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(uint32_t));
        return py::int_(val);
    });

    m.def("write_u32", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_u32");
        l_controller->thread_check_exception();
        uint32_t val = obj.cast<uint32_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(uint32_t));
    });

    m.def("read_u64", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_u64");
        l_controller->thread_check_exception();
        uint64_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(uint64_t));
        return py::int_(val);
    });

    m.def("write_u64", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_u64");
        l_controller->thread_check_exception();
        uint64_t val = obj.cast<uint64_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(uint64_t));
    });

    m.def("read_s8", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_s8");
        l_controller->thread_check_exception();
        int8_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(int8_t));
        return py::int_(val);
    });

    m.def("write_s8", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_s8");
        l_controller->thread_check_exception();
        int8_t val = obj.cast<int8_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(int8_t));
    });
    
    m.def("read_s16", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_s16");
        l_controller->thread_check_exception();
        int16_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(int16_t));
        return py::int_(val);
    });

    m.def("write_s16", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_s16");
        l_controller->thread_check_exception();
        int16_t val = obj.cast<int16_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(int16_t));
    });

    m.def("read_s32", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_s32");
        l_controller->thread_check_exception();
        int32_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(int32_t));
        return py::int_(val);
    });

    m.def("write_s32", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_s32");
        l_controller->thread_check_exception();
        int32_t val = obj.cast<int32_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(int32_t));
    });
    
    m.def("read_s64", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_s64");
        l_controller->thread_check_exception();
        int64_t val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(int64_t));
        return py::int_(val);
    });

    m.def("write_s64", [](int32_t ptr, py::int_ obj){
        ZoneScopedN("_recomp_mem_managed.write_s64");
        l_controller->thread_check_exception();
        int64_t val = obj.cast<int64_t>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(int64_t));
    });
    
    m.def("read_f32", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_f32");
        l_controller->thread_check_exception();
        float val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(float));
        return py::float_(val);
    });

    m.def("write_f32", [](int32_t ptr, py::float_ obj){
        ZoneScopedN("_recomp_mem_managed.write_f32");
        l_controller->thread_check_exception();
        float val = obj.cast<float>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(float));
    });
    
    m.def("read_f64", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_f64");
        l_controller->thread_check_exception();
        double val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(double));
        return py::float_(val);
    });

    m.def("write_f64", [](int32_t ptr, py::float_ obj){
        ZoneScopedN("_recomp_mem_managed.write_f64");
        l_controller->thread_check_exception();
        double val = obj.cast<double>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(double));
    });
    
    // Text
    m.def("read_char", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_char");
        l_controller->thread_check_exception();
        char val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(char));
        return py::str(&val, 1);
    });

    m.def("write_char", [](int32_t ptr, py::str obj){
        ZoneScopedN("_recomp_mem_managed.write_char");
        l_controller->thread_check_exception();
        char val = obj.cast<char>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(char));
    });

    m.def("read_byte_char", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_byte_char");
        l_controller->thread_check_exception();
        char val;
        memcpy_rev_from_recomp(g_controller->get_rdram(), &val, ptr, sizeof(char));
        return py::bytes(&val, 1);
    });

    m.def("write_byte_char", [](int32_t ptr, py::bytes obj){
        ZoneScopedN("_recomp_mem_managed.write_byte_char");
        l_controller->thread_check_exception();
        char val = obj.cast<char>();
        memcpy_rev_to_recomp(g_controller->get_rdram(), ptr, &val, sizeof(char));
    });

    m.def("read_str", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_str");
        l_controller->thread_check_exception();
        std::u8string str = ptr_to_u8string(g_controller->get_rdram(), ptr);
        return py::str(str);
    });

    m.def("read_str_n", [](int32_t ptr, uint32_t size){
        ZoneScopedN("_recomp_mem_managed.read_str_n");
        l_controller->thread_check_exception();
        std::u8string str = ptr_to_u8string_n(g_controller->get_rdram(), size, ptr);
        return py::str(str);
    });

    m.def("write_str_n", [](int32_t ptr, py::str str, uint32_t size){
        ZoneScopedN("_recomp_mem_managed.write_str_n");
        l_controller->thread_check_exception();
        uint8_t* rdram = g_controller->get_rdram(); // Used by MEM_B
        
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
            FrameMark;
        }
    });

    m.def("read_byte_str", [](int32_t ptr){
        ZoneScopedN("_recomp_mem_managed.read_byte_str");
        l_controller->thread_check_exception();
        std::string str = ptr_to_string(g_controller->get_rdram(), ptr);
        return py::bytes(str);
    });

    m.def("read_byte_str_n", [](int32_t ptr, uint32_t size){
        ZoneScopedN("_recomp_mem_managed.read_byte_str_n");
        l_controller->thread_check_exception();
        std::string str = ptr_to_string_n(g_controller->get_rdram(), size, ptr);
        return py::bytes(str);
    });

    m.def("write_byte_str_n", [](int32_t ptr, py::bytes str, uint32_t size){
        ZoneScopedN("_recomp_mem_managed.write_byte_str_n");
        l_controller->thread_check_exception();
        uint8_t* rdram = g_controller->get_rdram(); // Used by MEM_B
        
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
            FrameMark;
        }
    });
    
    // Memory Blocks:
    m.def("read_bytes_n", [](int32_t ptr, uint32_t size, bool reverse) {
        ZoneScopedN("_recomp_mem_managed.read_bytes_n");
        l_controller->thread_check_exception();
        uint8_t* rdram = g_controller->get_rdram();
        uint8_t* buf = new uint8_t[size];

        if (reverse) {
            memcpy_rev_from_recomp(rdram, buf, ptr, size);
        } else {
            memcpy_from_recomp(rdram, buf, ptr, size);
        }

        py::bytes retVal = py::bytes((char*)buf, size);
        delete[] buf;

        return retVal;
    });

    m.def("read_bytearray_n", [](int32_t ptr, uint32_t size, bool reverse) {
        ZoneScopedN("_recomp_mem_managed.read_bytearray_n");
        l_controller->thread_check_exception();
        uint8_t* rdram = g_controller->get_rdram();

        uint8_t* buf = new uint8_t[size];

        if (reverse) {
            memcpy_rev_from_recomp(rdram, buf, ptr, size);
        } else {
            memcpy_from_recomp(rdram, buf, ptr, size);
        }

        py::bytearray retVal = py::bytearray((char*)buf, size);
        delete[] buf;

        return retVal;
    });

    m.def("write_buffer_n", [](int32_t ptr, py::buffer buffer, uint32_t size, bool reverse) {
        ZoneScopedN("_recomp_mem_managed.write_buffer_n");
        l_controller->thread_check_exception();
        uint8_t* rdram = g_controller->get_rdram(); // Used by MEM_B
        
        // Accessing underlying byte array.
        py::buffer_info info = buffer.request();
        uint8_t* buf_data = (uint8_t*)info.ptr;
        py::ssize_t buf_size = std::min(info.size, (py::ssize_t)size); 

        if (reverse) {
            memcpy_rev_to_recomp(rdram, ptr, buf_data, buf_size);
        } else {
            memcpy_to_recomp(rdram, ptr, buf_data, buf_size);
        }
    });
}


// Direct access to recomp memory without byteswapping.
PYBIND11_EMBEDDED_MODULE(_recomp_mem_raw, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
    ZoneScoped

    m.def("create_raw_memoryview", [](int32_t ptr, uint32_t size){
        ZoneScopedN("_recomp_mem_raw.create_raw_memoryview");
        l_controller->thread_check_exception();
        void* rptr = RDRAM_TO_PTR(g_controller->get_rdram(), void, ptr);
        return py::memoryview::from_memory(rptr, size);
    });
}