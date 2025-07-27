#pragma once
#include "globals.hpp"

#define PYMAP_LEVEL_SIZE 256

typedef unsigned int REPY_Handle;

struct REPY_HandleEntry {
    py::object py_object = py::none();
    bool is_single_use = false;
};

#define PY_SPLIT_HANDLE(handle, level1_name, level2_name, level3_name, level4_name) \
uint8_t level1_name = ((handle & 0xFF000000) >> 24); \
uint8_t level2_name = ((handle & 0x00FF0000) >> 16); \
uint8_t level3_name = ((handle & 0x0000FF00) >> 8); \
uint8_t level4_name = ((handle & 0x000000FF))

#define PY_SPLIT_HANDLE_DEFAULT(handle) \
PY_SPLIT_HANDLE(handle, l1, l2, l3, l4)

#define PY_HANDLE_LEVEL1_POS(handle) 0
#define PY_HANDLE_LEVEL2_POS(handle) (handle & 0xFF000000)
#define PY_HANDLE_LEVEL3_POS(handle) (handle & 0xFFFF0000)
#define PY_HANDLE_LEVEL4_POS(handle) (handle & 0xFFFFFF00)

class PyObjectSlotMap {
public:
    struct MapLevel4 {
        uint32_t pos = 0;
        uint32_t count = 0;
        REPY_HandleEntry* level4[PYMAP_LEVEL_SIZE];
        MapLevel4(uint32_t p_pos);
        ~MapLevel4();
    };

    struct MapLevel3 {
        uint32_t pos = 0;
        uint32_t count = 0;
        MapLevel4* level3[PYMAP_LEVEL_SIZE];
        MapLevel3(uint32_t p_pos);
        ~MapLevel3();
    };

    struct MapLevel2 {
        uint32_t pos = 0;
        uint32_t count = 0;
        MapLevel3* level2[PYMAP_LEVEL_SIZE];
        MapLevel2(uint32_t p_pos);
        ~MapLevel2();
    };

    PyObjectSlotMap();
    ~PyObjectSlotMap();
    uint32_t get_count();
    REPY_HandleEntry* get(REPY_Handle handle);
    REPY_Handle add(py::object* object);
    REPY_Handle add_and_steal(py::object* object);
    bool has(REPY_Handle handle);
    void del(REPY_Handle handle);
    void del_all();

private:
    // REPY_HandleEntry* lookup(REPY_Handle handle);
    REPY_Handle next_handle_val = 1;
    uint32_t level1_count = 0;
    uint32_t count = 0;
    MapLevel2* level1[255];
    bool wrapped_around = false;
    REPY_Handle get_next_handle();
};