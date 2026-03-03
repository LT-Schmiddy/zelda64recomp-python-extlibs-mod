"""
\defgroup python_api Python API: The `repy_api` Module
repy_api: This module access to certain N64Recomp information from Python code.

@{
"""

from . import mem
from . import utils

version_str = "2.0.0"

__all__ = [
    "mem",
    "utils"
]

## @}