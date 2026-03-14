## \ingroup py_mem
## @{
## \defgroup py_mem_byteswapped repy_api.mem.byteswapped
## @{

"""
repy_api.mem.byteswapped: This module provides easy access and wrappers to data inside N64Recompiled memory.

"""


import _recomp_mem_byteswapped

def read_u8(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_u8(ptr)
    
def write_u8(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_u8(ptr, value)

def read_u16(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_u16(ptr)
    
def write_u16(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_u16(ptr, value)

def read_u32(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_u32(ptr)
    
def write_u32(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_u32(ptr, value)

def read_u64(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_u64(ptr)
    
def write_u64(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_u64(ptr, value)

# Signed Ints:
def read_s8(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_s8(ptr)
    
def write_s8(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_s8(ptr, value)

def read_s16(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_s16(ptr)
    
def write_s16(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_s16(ptr, value)

def read_s32(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_s32(ptr)
    
def write_s32(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_s32(ptr, value)

def read_s64(ptr: int) -> int:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        int: _description_
    """
    return _recomp_mem_byteswapped.read_s64(ptr)
    
def write_s64(ptr: int, value: int):
    """_summary_

    Args:
        ptr (int): _description_
        value (int): _description_
    """
    _recomp_mem_byteswapped.write_s64(ptr, value)

# Floats:
def read_f32(ptr: int) -> float:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        float: _description_
    """
    return _recomp_mem_byteswapped.read_f32(ptr)
    
def write_f32(ptr: int, value: float):
    """_summary_

    Args:
        ptr (int): _description_
        value (float): _description_
    """
    _recomp_mem_byteswapped.write_f32(ptr, value)


def read_f64(ptr: int) -> float:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        float: _description_
    """
    return _recomp_mem_byteswapped.read_f64(ptr)
    
def write_f64(ptr: int, value: float):
    """_summary_

    Args:
        ptr (int): _description_
        value (float): _description_
    """
    _recomp_mem_byteswapped.write_f64(ptr, value)

# Text:
def read_char(ptr: int) -> str:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        str: _description_
    """
    return _recomp_mem_byteswapped.read_char(ptr)
    
def write_char(ptr: int, char: str):
    """_summary_

    Args:
        ptr (int): _description_
        char (str): _description_

    Raises:
        ValueError: _description_
    """
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_byteswapped.write_char(ptr, char)

def read_byte_char(ptr: int) -> bytes:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        bytes: _description_
    """
    return _recomp_mem_byteswapped.read_byte_char(ptr)
    
def write_byte_char(ptr: int, char: bytes):
    """_summary_

    Args:
        ptr (int): _description_
        char (bytes): _description_

    Raises:
        ValueError: _description_
    """
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_byteswapped.write_byte_char(ptr, char)

def read_str(ptr: int) -> str:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        str: _description_
    """
    return _recomp_mem_byteswapped.read_str(ptr)

def read_str_n(ptr: int, size: int) -> str:
    """_summary_

    Args:
        ptr (int): _description_
        size (int): _description_

    Returns:
        str: _description_
    """
    return _recomp_mem_byteswapped.read_str_n(ptr, size)

def write_str_n(ptr: int, data: str, size: int):
    """_summary_

    Args:
        ptr (int): _description_
        data (str): _description_
        size (int): _description_
    """
    _recomp_mem_byteswapped.write_str_n(ptr, data, size)

def read_byte_str(ptr: int) -> bytes:
    """_summary_

    Args:
        ptr (int): _description_

    Returns:
        bytes: _description_
    """
    return _recomp_mem_byteswapped.read_byte_str(ptr)

def read_byte_str_n(ptr: int, size: int) -> bytes:
    """_summary_

    Args:
        ptr (int): _description_
        size (int): _description_

    Returns:
        bytes: _description_
    """
    return _recomp_mem_byteswapped.read_byte_str_n(ptr, size)

def write_byte_str_n(ptr: int, data: str, size: int):
    """_summary_

    Args:
        ptr (int): _description_
        data (str): _description_
        size (int): _description_
    """
    _recomp_mem_byteswapped.write_byte_str_n(ptr, data, size)

# Memory
def read_bytes_n(ptr: int, size: int, reverse: bool = False) -> bytes:
    """_summary_

    Args:
        ptr (int): _description_
        size (int): _description_
        reverse (bool, optional): _description_. Defaults to False.

    Returns:
        bytes: _description_
    """
    return _recomp_mem_byteswapped.read_bytes_n(ptr, size, reverse)
    
def read_bytearray_n(ptr: int, size: int, reverse: bool = False) -> bytearray:
    """_summary_

    Args:
        ptr (int): _description_
        size (int): _description_
        reverse (bool, optional): _description_. Defaults to False.

    Returns:
        bytearray: _description_
    """
    return _recomp_mem_byteswapped.read_bytearray_n(ptr, size, reverse)
    
def write_buffer_n(ptr: int, data: bytearray, size: int, reverse: bool = False):
    """_summary_

    Args:
        ptr (int): _description_
        data (bytearray): _description_
        size (int): _description_
        reverse (bool, optional): _description_. Defaults to False.
    """
    _recomp_mem_byteswapped.write_buffer_n(ptr, data, size, reverse)
    
## @}
## @}