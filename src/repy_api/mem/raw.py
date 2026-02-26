"""
repy_api.mem.managed: This module provides direct access to data N64Recompiled memory.

"""

import _recomp_mem_raw

from .. import utils

def create_raw_memoryview(ptr: int, size: int) -> memoryview:
    return _recomp_mem_raw.create_raw_memoryview(ptr, size)

