## \ingroup python_api
# @{
# \defgroup py_mem repy_api.mem
# repy_api.mem: This module access to N64Recompiled memory from within Python code.
# 
# Recompiled memory addresses (sometimes referred to as pointers in this documentation) are represented with 
# Python `int` objects. 
# 
# @{

from . import byteswapped, raw

__all__ = [
    "byteswapped",
    "raw"
]
## @}
## @}