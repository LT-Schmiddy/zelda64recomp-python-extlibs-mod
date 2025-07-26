#include "pyobject_slotmap.hpp"
// Handling the submaps construction/destruction:
PyObjectSlotMap::MapLevel4::MapLevel4(uint32_t p_pos) {
    pos = p_pos;
    PLOGD.printf("MapLevel4 %08X created", pos);
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        level4[i] = NULL;
    }
}

PyObjectSlotMap::MapLevel4::~MapLevel4() {
    if (count != 0) {
        PLOGD.printf("MapLevel3 %08X deleted without being empty. %u items remaining. Cleaning now...", pos, count);
        for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
            delete level4[i];
            level4[i] = NULL;
        }
    } else {
        PLOGD.printf("MapLevel4 %08X deleted (empty).", pos);
    }
}

PyObjectSlotMap::MapLevel3::MapLevel3(uint32_t p_pos) {
    pos = p_pos;
    PLOGD.printf("MapLevel3 %08X created", pos);
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        level3[i] = NULL;
    }
}

PyObjectSlotMap::MapLevel3::~MapLevel3() {
    if (count != 0) {
        PLOGD.printf("MapLevel3 %08X deleted without being empty. %u items remaining. Cleaning now...", pos, count);
        for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
            delete level3[i];
            level3[i] = NULL;
        }
    } else {
        PLOGD.printf("MapLevel3 %08X deleted (empty).", pos);
    }
}

PyObjectSlotMap::MapLevel2::MapLevel2(uint32_t p_pos) {
    pos = p_pos;
    PLOGD.printf("MapLevel2 %08X created", pos);
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        level2[i] = NULL;
    }
}

PyObjectSlotMap::MapLevel2::~MapLevel2() {
    if (count != 0) {
        PLOGD.printf("MapLevel2 %08X deleted without being empty. %u items remaining. Cleaning now...", pos, count);
        for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
            delete level2[i];
            level2[i] = NULL;
        }
    } else {
        PLOGD.printf("MapLevel2 %08X deleted (empty).", pos);
    }
}

// The main map object:
PyObjectSlotMap::PyObjectSlotMap() {
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        level1[i] = NULL;
    }
}

PyObjectSlotMap::~PyObjectSlotMap() {
if (level1_count != 0) {
        PLOGD.printf("PyObjectSlotMap deleted without being empty. %u items remaining. Cleaning now...", level1_count);
        for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
            delete level1[i];
            level1[i] = NULL;
        }
    } else {
        PLOGD.printf("PyObjectSlotMap deleted (empty).");
    }
}

uint32_t PyObjectSlotMap::get_count() {
    return count;
}

REPY_HandleEntry* PyObjectSlotMap::get(REPY_Handle handle) {
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapLevel2* level2 = level1[l1];
    if (level2 == NULL) {
        PLOGV.printf("MapLevel2 %02X of handle %08X is NULL", l1, handle);
        return NULL;
    }

    MapLevel3* level3 = level2->level2[l2];
    if (level3 == NULL) {
        PLOGV.printf("MapLevel3 %02X of handle %08X is NULL", l2, handle);
        return NULL;
    }

    MapLevel4* level4 = level3->level3[l3];
    if (level4 == NULL) {
        PLOGV.printf("MapLevel4 %02X of handle %08X is NULL", l3, handle);
        return NULL;
    }

    REPY_HandleEntry* entry = level4->level4[l4];
    if (entry == NULL) {
        PLOGV.printf("REPY_HandleEntry %02X of handle %08X is NULL", l4, handle);
        return NULL;
    }

    return entry;
}

REPY_Handle PyObjectSlotMap::add(py::object* object) {
    REPY_Handle handle = get_next_handle();
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapLevel2* level2 = level1[l1];
    if (level2 == NULL) {
        level2 = new MapLevel2(PY_HANDLE_LEVEL2_POS(handle));
        level1[l1] = level2;
    } else {
        level2->count++;
    }

    MapLevel3* level3 = level2->level2[l2];
    if (level3 == NULL) {
        level3 = new MapLevel3(PY_HANDLE_LEVEL3_POS(handle));
        level2->level2[l2] = level3;
    }else {
        level3->count++;
    }

    MapLevel4* level4 = level3->level3[l3];
    if (level4 == NULL) {
        level4 = new MapLevel4(PY_HANDLE_LEVEL4_POS(handle));
        level3->level3[l3] = level4;
    }else {
        level4->count++;
    }

    REPY_HandleEntry* entry = level4->level4[l4];
    if (entry == NULL) {
        entry = new REPY_HandleEntry {(*object), false};
        level4->level4[l4] = entry;
    } 
    count++;
    return handle;
}

REPY_Handle PyObjectSlotMap::add_and_steal(py::object* object) {
    REPY_Handle handle = get_next_handle();
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapLevel2* level2 = level1[l1];
    if (level2 == NULL) {
        level2 = new MapLevel2(PY_HANDLE_LEVEL2_POS(handle));
        level1[l1] = level2;
    } else {
        level2->count++;
    }

    MapLevel3* level3 = level2->level2[l2];
    if (level3 == NULL) {
        level3 = new MapLevel3(PY_HANDLE_LEVEL3_POS(handle));
        level2->level2[l2] = level3;
    }else {
        level3->count++;
    }

    MapLevel4* level4 = level3->level3[l3];
    if (level4 == NULL) {
        level4 = new MapLevel4(PY_HANDLE_LEVEL4_POS(handle));
        level3->level3[l3] = level4;
    }else {
        level4->count++;
    }

    assert(level4->level4[l4] == NULL);
    REPY_HandleEntry* entry = new REPY_HandleEntry {py::reinterpret_steal<py::object>(*object), false};
    level4->level4[l4] = entry;
    
    count++;
    return handle;
}

bool PyObjectSlotMap::has(REPY_Handle handle) {
    return get(handle) == NULL;
}

void PyObjectSlotMap::del(REPY_Handle handle) {
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapLevel2* level2 = level1[l1];
    assert(level2 != NULL);
    MapLevel3* level3 = level2->level2[l2];
    assert(level3 != NULL);
    MapLevel4* level4 = level3->level3[l3];
    assert(level4 != NULL);
    REPY_HandleEntry* entry = level4->level4[l4];
    assert(entry != NULL);

    delete entry;
    level4->count--;
    if (level4->count == 0) {
        delete level4;
        level3->count--;
        if (level3->count == 0) {
            delete level3;
            level2->count--;
            if (level2->count == 0) {
                delete level2;
                level1_count--;
            }
        }
    }
    count--;
}

REPY_Handle PyObjectSlotMap::get_next_handle() {
    while (has(next_handle_val) || next_handle_val == 0) {
        next_handle_val++;
    }
    return next_handle_val++;
}