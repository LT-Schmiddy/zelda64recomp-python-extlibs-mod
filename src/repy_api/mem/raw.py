import _recomp_mem_raw

from .. import utils

def create_raw_memoryview(ptr: int, size: int) -> memoryview:
    return _recomp_mem_raw.create_raw_memoryview(ptr, size)

