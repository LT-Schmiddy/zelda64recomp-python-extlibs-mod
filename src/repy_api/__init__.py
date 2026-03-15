"""
@defgroup python_api Python API: The repy_api Python Module
@brief `repy_api`: This module access to certain N64Recomp information from Python code.

@{
"""

from . import mem


## \var version_str
# @brief The current version of REPY.
version_str = "2.0.0"

__all__ = [
    "mem"
]

## @}