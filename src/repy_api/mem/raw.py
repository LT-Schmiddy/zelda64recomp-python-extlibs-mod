## \ingroup py_mem
## @{
## \defgroup py_mem_raw repy_api.mem.raw
## @{


"""
repy_api.mem.raw: This module provides direct access to data N64Recompiled memory.

"""

import _recomp_mem_raw

from .. import utils

def create_raw_memoryview(ptr: int, size: int) -> memoryview:
    """_summary_

    Args:
        ptr (int): _description_
        size (int): _description_

    Returns:
        memoryview: _description_
    """
    return _recomp_mem_raw.create_raw_memoryview(ptr, size)

