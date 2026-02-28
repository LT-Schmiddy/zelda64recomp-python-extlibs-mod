## \ingroup python_api
## @{
## \defgroup py_utils repy_api.utils
## @{

"""
repy_api: This module access to certain N64Recomp information from Python code.

"""

import threading

def is_main_thread() -> bool:
    """_summary_

    Returns:
        bool: _description_
    """
    return threading.current_thread() is threading.main_thread()

def assert_main_thread():
    """_summary_

    Raises:
        RuntimeError: _description_
    """
    if not is_main_thread():
        raise RuntimeError("This function can only be called from the main thread.")
    
    
## @}
## @}