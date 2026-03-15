## \ingroup py_mem
# @{
# 
# \defgroup py_mem_byteswapped repy_api.mem.byteswapped
# @brief `repy_api.mem.byteswapped`: This module provides easy access and wrappers to data inside N64Recompiled memory.
#
# Due to the N64 being a big-endian system, and most modern machines being little-endian,
#
# Note that these functions can only be called from threads created by N64Recompiled. Attempting to call them from other
# threads (such as threads created with `threading.Thread`) will throw a `RuntimeError`.
# @{
    
import _recomp_mem_byteswapped

def read_u8(ptr: int) -> int:
    """@brief Read an unsigned, 8-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_u8(ptr)
    
def write_u8(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 8-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_u8(ptr, value)

def read_u16(ptr: int) -> int:
    """@brief Read an unsigned, 16-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_u16(ptr)
    
def write_u16(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 16-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_u16(ptr, value)

def read_u32(ptr: int) -> int:
    """@brief Read an unsigned, 32-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_u32(ptr)
    
def write_u32(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 32-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_u32(ptr, value)

def read_u64(ptr: int) -> int:
    """@brief Read an unsigned, 64-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_u64(ptr)
    
def write_u64(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 32-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_u64(ptr, value)

# Signed Ints:
def read_s8(ptr: int) -> int:
    """@brief Read a signed, 8-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_s8(ptr)
    
def write_s8(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 8-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_s8(ptr, value)

def read_s16(ptr: int) -> int:
    """@brief Read a signed, 16-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_s16(ptr)
    
def write_s16(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 16-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_s16(ptr, value)

def read_s32(ptr: int) -> int:
    """@brief Read a signed, 32-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_s32(ptr)
    
def write_s32(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 32-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_s32(ptr, value)

def read_s64(ptr: int) -> int:
    """@brief Read a signed, 64-bit integer from recompiled memory and cast it as a Python `int` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_s64(ptr)
    
def write_s64(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 64-bit integer.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_s64(ptr, value)

# Floats:
def read_f32(ptr: int) -> float:
    """@brief Read a 32-bit floating point value from recompiled memory and cast it as a Python `float` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `float` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_f32(ptr)
    
def write_f32(ptr: int, value: float):
    """@brief Write a Python `float` object to recompiled memory as a 32-bit floating point value.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_f32(ptr, value)


def read_f64(ptr: int) -> float:
    """@brief Read a 64-bit floating point value from recompiled memory and cast it as a Python `float` type.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `float` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_f64(ptr)
    
def write_f64(ptr: int, value: float):
    """@brief Write a Python `float` object to recompiled memory as a 64-bit floating point value.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_f64(ptr, value)

# Text:
def read_char(ptr: int) -> str:
    """@brief Read a single character from recompiled memory and cast it to a Python `str` object.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_char(ptr)
    
def write_char(ptr: int, char: str):
    """@brief Write a single character represented by a `str` object to recompiled memory.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    @throws ValueError if the length of `char` is greater than 1.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_byteswapped.write_char(ptr, char)

def read_byte_char(ptr: int) -> bytes:
    """@brief Read a single character from recompiled memory and cast it to a Python `bytes` object.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_byte_char(ptr)
    
def write_byte_char(ptr: int, char: bytes):
    """@brief Write a single character represented by a `bytes` object to recompiled memory.
    
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    @throws ValueError if the length of `char` is greater than 1.

    @param ptr The mod memory address to write to from. 
    @param value The integer value to write. 
    """
    _recomp_mem_byteswapped.write_byte_char(ptr, char)

def read_str(ptr: int) -> str:
    """@brief Read a null-terminated string from recompiled memory and cast it as a Python `str` object.

    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`
    """
    return _recomp_mem_byteswapped.read_str(ptr)

def read_str_n(ptr: int, size: int) -> str:
    """@brief Read a string from recompiled memory and cast it as a Python `str` object. End with null-termination or when `size` is reached.

    @throws RuntimeError if called from a thread not created by N64Recompiled.

    @param ptr The mod memory address to read from. 
    @param size The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`
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