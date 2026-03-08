## \ingroup python_api
## @{
## \defgroup py_mem repy_api.mem
## @{

"""
repy_api.mem: This module access to N64Recompiled memory from within Python code.

"""

from . import byteswapped, raw

__all__ = [
    "byteswapped",
    "raw"
]