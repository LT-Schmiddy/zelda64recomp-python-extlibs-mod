## \ingroup py_mem
# @{
# \defgroup py_mem_raw repy_api.mem.raw
# @brief `repy_api.mem.raw`: This module provides direct access to data N64Recompiled memory, without any kind of byteswapping or type-casting.
# 
# 
# @{


import _recomp_mem_raw

def create_raw_memoryview(ptr: int, size: int) -> memoryview:
    """@brief Create a Python `memoryview` object granting direct, raw access to recompiled memory.
    
    Be advised that using a memoryview on a non-N64Recompiled thread may be unsafe.

    @param ptr The mod memory address to write to. 
    @param size The maximum size of the memory to write.
    @return a Python `memoryview` object wrapping a region of recompiled memory.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_raw.create_raw_memoryview(ptr, size)

## @}
## @}