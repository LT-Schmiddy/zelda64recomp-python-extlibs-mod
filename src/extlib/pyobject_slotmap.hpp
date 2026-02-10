#pragma once
#include "globals.hpp"

#define PYMAP_LEVEL_SIZE 65536

struct REPY_HandleEntry {
    py::object py_object = py::none();
    bool is_single_use = false;
    REPY_InterpreterHandle interp_index = 0;
};

#define PY_SPLIT_HANDLE(handle, page_index_name, entry_index_name) \
uint16_t page_index_name = ((handle & 0xFFFF0000) >> 16); \
uint16_t entry_index_name = ((handle & 0x0000FFFF)); 


#define PY_SPLIT_HANDLE_DEFAULT(handle) \
PY_SPLIT_HANDLE(handle, page_index, entry_index)

#define PY_HANDLE_PAGE_TABLE_POS(handle) (handle & 0xFFFF0000)


class PyObjectSlotMap {
public:
    struct MapPage {
        uint32_t pos = 0;
        uint32_t count = 0;
        REPY_HandleEntry* entries[PYMAP_LEVEL_SIZE];
        MapPage(uint32_t p_pos);
        ~MapPage();
    };

    PyObjectSlotMap();
    ~PyObjectSlotMap();
    uint32_t get_count();
    REPY_HandleEntry* get(REPY_Handle handle);
    REPY_Handle add(py::object* object, REPY_InterpreterHandle sub_index);
    bool has(REPY_Handle handle);
    void del(REPY_Handle handle);
    void del_all();

private:
    REPY_Handle next_handle_val = 1;
    uint32_t page_count = 0;
    uint32_t count = 0;
    MapPage* pages[PYMAP_LEVEL_SIZE];
    bool wrapped_around = false;
    REPY_Handle get_next_handle();
};