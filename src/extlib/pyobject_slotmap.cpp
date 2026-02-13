#include "pyobject_slotmap.hpp"

// Handling the submaps construction/destruction:
PyObjectSlotMap::MapPage::MapPage(uint32_t p_pos) {
    ZoneScoped;
    pos = p_pos;
    PLOGD.printf("MapPage 0x%08X created", pos);
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        entries[i] = NULL;
    }
}

PyObjectSlotMap::MapPage::~MapPage() {
    ZoneScoped;
    if (count != 0) {
        PLOGD.printf("MapPage 0x%08X deleted without being empty. %u items remaining. Cleaning now...", pos, count);
        for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
            delete entries[i];
            entries[i] = NULL;
        }
    } else {
        PLOGD.printf("MapPage 0x%08X deleted (empty).", pos);
    }
}

// The main map object:
PyObjectSlotMap::PyObjectSlotMap() {
    ZoneScoped;
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        pages[i] = NULL;
    }
}

PyObjectSlotMap::~PyObjectSlotMap() {
    ZoneScoped;
    if (page_count != 0) {
        PLOGD.printf("PyObjectSlotMap deleted without being empty. %u items remaining. Cleaning now...", page_count);

    } else {
        PLOGD.printf("PyObjectSlotMap deleted (empty).");
    }
}

uint32_t PyObjectSlotMap::get_count() {
    ZoneScoped;
    return count;
}

REPY_HandleEntry* PyObjectSlotMap::get(REPY_Handle handle) {
    ZoneScoped;
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapPage* page = pages[page_index];
    if (page == NULL) {
        PLOGV.printf("Page index 0x%04X of handle 0x%08X is NULL", page_index, handle);
        return NULL;
    }

    REPY_HandleEntry* entry = page->entries[entry_index];
    if (entry == NULL) {
        PLOGV.printf("Entry index 0x%04X of handle 0x%08X is NULL", entry_index, handle);
        return NULL;
    }

    return entry;
}

REPY_Handle PyObjectSlotMap::add(py::object* object, REPY_InterpreterHandle sub_index) {
    ZoneScoped;
    REPY_Handle handle = get_next_handle();
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapPage* page = pages[page_index];
    if (page == NULL) {
        page = new MapPage(PY_HANDLE_PAGE_TABLE_POS(handle));
        pages[page_index] = page;
        page_count++;
    }

    assert(page->entries[entry_index] == NULL);
    REPY_HandleEntry* entry = new REPY_HandleEntry {(*object), false, sub_index};
    page->entries[entry_index] = entry;
    page->count++;
    count++;
    return handle;
}

bool PyObjectSlotMap::has(REPY_Handle handle) {
    ZoneScoped;
    return get(handle) != NULL;
}

void PyObjectSlotMap::del(REPY_Handle handle) {
    ZoneScoped;
    PY_SPLIT_HANDLE_DEFAULT(handle);

    MapPage* page = pages[page_index];
    assert(page != NULL);
    REPY_HandleEntry* entry = page->entries[entry_index];
    assert(entry != NULL);

    REPY_Handle page_max_handle = page->pos + 0xFFFF;
    bool preserve_page = false;

    // Don't delete if we KNOW we're gonna allocate on this level4 again. 
    if (page->pos <= next_handle_val && next_handle_val <= page_max_handle) {
        preserve_page = true;
    }

    delete entry;
    page->entries[entry_index] = NULL;
    page->count--;
    if (page->count == 0 && !preserve_page) {
        delete page;
        pages[page_index] = NULL;
        page_count--;
    }
    count--;
}

REPY_Handle PyObjectSlotMap::get_next_handle() {
    ZoneScoped;
    PY_SPLIT_HANDLE_DEFAULT(next_handle_val);
    if (entry_index == 0) {
        // We're moving on to a new page, so check if the old page exists and is empty. Delete it if so.
        // Important to prevent memory leaks
        MapPage* page = pages[page_index];
        if (page != NULL && page->count == 0) {
            delete page;
            pages[page_index] = NULL;
            page_count--;
        }
    }

    // next_handle_val starts at 1. The only way to reach 0 is by integer overflow,
    // which means we need to start checking slots are already in use.
    if (next_handle_val == 0) {
        wrapped_around = true;
    }

    while (wrapped_around && (next_handle_val == 0 || has(next_handle_val))) {
        next_handle_val++;
    }
    REPY_Handle retVal = next_handle_val;
    next_handle_val++;
    return retVal;
}

void PyObjectSlotMap::del_all() {
    ZoneScoped;
    for (uint32_t i = 0; i < PYMAP_LEVEL_SIZE; i++) {
        delete pages[i];
        pages[i] = NULL;
    }
}