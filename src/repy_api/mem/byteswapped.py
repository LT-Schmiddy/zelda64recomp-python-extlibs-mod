## \ingroup py_mem
# @{
# 
# \defgroup py_mem_byteswapped repy_api.mem.byteswapped
# @brief `repy_api.mem.byteswapped`: This module provides easy reading and writing of data inside N64Recompiled memory.
#
# Due to the N64 being a big-endian system and most modern machines being little-endian (and more importantly the optimizations
# that N64Recompiled makes to compensate for this difference, which are beyond the scope of this article), recompiled memory
# can often appear garbled on the native side, and byteswapping is often necessary when moving data in and out.
#
# The function in this module handle all of that for you, swapping bytes while data is being copied to and from recompiled memory,
# ensuring that N64Recompiled and Python have the same data in the format they expect. This 'copy and swap' is implemented in C++
# for maximum performance. That being said, these are still copy operations. If you need to manipulate recompiled memory directly,
# look at \ref py_mem_raw.
#
# Note that these functions can only be called from threads created by N64Recompiled. Attempting to call them from other
# threads (such as threads created with `threading.Thread`) will throw a `RuntimeError`.
# @{\

from collections.abc import Buffer
import _recomp_mem_byteswapped

def read_u8(ptr: int) -> int:
    """@brief Read an unsigned, 8-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_u8(ptr)
    
def write_u8(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 8-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_u8(ptr, value)

def read_u16(ptr: int) -> int:
    """@brief Read an unsigned, 16-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_u16(ptr)
    
def write_u16(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 16-bit integer.
    
    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_u16(ptr, value)

def read_u32(ptr: int) -> int:
    """@brief Read an unsigned, 32-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_u32(ptr)
    
def write_u32(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 32-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_u32(ptr, value)

def read_u64(ptr: int) -> int:
    """@brief Read an unsigned, 64-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_u64(ptr)
    
def write_u64(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as an unsigned 32-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_u64(ptr, value)

# Signed Ints:
def read_s8(ptr: int) -> int:
    """@brief Read a signed, 8-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_s8(ptr)
    
def write_s8(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 8-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_s8(ptr, value)

def read_s16(ptr: int) -> int:
    """@brief Read a signed, 16-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_s16(ptr)
    
def write_s16(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 16-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_s16(ptr, value)

def read_s32(ptr: int) -> int:
    """@brief Read a signed, 32-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_s32(ptr)
    
def write_s32(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 32-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_s32(ptr, value)

def read_s64(ptr: int) -> int:
    """@brief Read a signed, 64-bit integer from recompiled memory and cast it as a Python `int` type.

    @param ptr The mod memory address to read from. 
    @return a Python `int` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_s64(ptr)
    
def write_s64(ptr: int, value: int):
    """@brief Write a Python `int` object to recompiled memory as a signed 64-bit integer.

    @param ptr The mod memory address to write to. 
    @param value The integer value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_s64(ptr, value)

# Floats:
def read_f32(ptr: int) -> float:
    """@brief Read a 32-bit floating point value from recompiled memory and cast it as a Python `float` type.

    @param ptr The mod memory address to read from. 
    @return a Python `float` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_f32(ptr)
    
def write_f32(ptr: int, value: float):
    """@brief Write a Python `float` object to recompiled memory as a 32-bit floating point value.

    @param ptr The mod memory address to write to. 
    @param value The floating point value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_f32(ptr, value)


def read_f64(ptr: int) -> float:
    """@brief Read a 64-bit floating point value from recompiled memory and cast it as a Python `float` type.

    @param ptr The mod memory address to read from. 
    @return a Python `float` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_f64(ptr)
    
def write_f64(ptr: int, value: float):
    """@brief Write a Python `float` object to recompiled memory as a 64-bit floating point value.

    @param ptr The mod memory address to write to. 
    @param value The floating point value to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_f64(ptr, value)

# Text:
def read_char(ptr: int) -> str:
    """@brief Read a single character from recompiled memory and cast it to a Python `str` object.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_char(ptr)
    
def write_char(ptr: int, char: str):
    """@brief Write a single character represented by a `str` object to recompiled memory.

    @param ptr The mod memory address to write to. 
    @param char The character to write, expressed as a `str` object. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    @throws ValueError if the length of `char` is greater than 1.
    """
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_byteswapped.write_char(ptr, char)

def read_byte_char(ptr: int) -> bytes:
    """@brief Read a single character from recompiled memory and cast it to a Python `bytes` object.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the value read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_byte_char(ptr)
    
def write_byte_char(ptr: int, char: bytes):
    """@brief Write a single character represented by a `bytes` object to recompiled memory.

    @param ptr The mod memory address to write to. 
    @param char The character to write, expressed as a `bytes` object. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    @throws ValueError if the length of `char` is greater than 1.
    """
    _recomp_mem_byteswapped.write_byte_char(ptr, char)

def read_str(ptr: int) -> str:
    """@brief Read a null-terminated string from recompiled memory and cast it as a Python `str` object.

    @param ptr The mod memory address to read from. 
    @return a Python `str` with the content read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_str(ptr)

def read_str_n(ptr: int, size: int) -> str:
    """@brief Read a string from recompiled memory and cast it as a Python `str` object. Ends with null-termination or when `size` is reached.

    @param ptr The mod memory address to read from. 
    @param size The maximum size of the memory to read. 
    @return a Python `str` with the content read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_str_n(ptr, size)

def write_str_n(ptr: int, data: str, size: int):
    """@brief Write a Python `str` object to recompiled memory as a null-terminated string. If the `str` object is larger than `size`, it will be truncated.

    @param ptr The mod memory address to write to. 
    @param data A Python `str` object to write to mod memory.
    @param size The maximum size of the memory to write.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_str_n(ptr, data, size)

def read_byte_str(ptr: int) -> bytes:
    """@brief Read a null-terminated string from recompiled memory and cast it as a Python `bytes` object.

    @param ptr The mod memory address to read from. 
    @return a Python `bytes` with the content read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_byte_str(ptr)

def read_byte_str_n(ptr: int, size: int) -> bytes:
    """@brief Read a string from recompiled memory and cast it as a Python `bytes` object. Ends with null-termination or when `size` is reached.

    @param ptr The mod memory address to read from. 
    @param size The maximum size of the memory to read. 
    @return a Python `bytes` with the content read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_byte_str_n(ptr, size)

def write_byte_str_n(ptr: int, data: str, size: int):
    """@brief Write a Python `bytes` object to recompiled memory as a null-terminated string. If the `bytes` object is larger than `size`,
    or if the null byte is reached before the end of the `bytes` object, the content it will be truncated.

    @param ptr The mod memory address to write to. 
    @param data A Python `bytes` object to write to mod memory.
    @param size The maximum size of the memory to write. 
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_byte_str_n(ptr, data, size)

# Memory
def read_bytes_n(ptr: int, size: int, reverse: bool = False) -> bytes:
    """@brief read a region of recompiled memory of `size` bytes into the Python interpreter as a `bytes` object.
    
    Unlike `read_byte_str` and `read_byte_str_n`, this function copies the memory region exactly as it is, null bytes
    and all.

    @param ptr The mod memory address to read from. 
    @param size The size of the memory to read. 
    @param reverse If True, mirrors the memory being read. Defaults to False.
    @return a Python `bytes` object with the data read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_bytes_n(ptr, size, reverse)
    
def read_bytearray_n(ptr: int, size: int, reverse: bool = False) -> bytearray:
    """@brief read a region of recompiled memory of `size` bytes into the Python interpreter as a `bytearray` object.
    
    Unlike `read_byte_str` and `read_byte_str_n`, this function copies the memory region exactly as it is, null bytes
    and all.

    @param ptr The mod memory address to read from. 
    @param size The size of the memory to read. 
    @param reverse If True, mirrors the memory being read. Defaults to False
    @return a Python `bytearray` object with the data read from `ptr`.
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    return _recomp_mem_byteswapped.read_bytearray_n(ptr, size, reverse)
    
def write_buffer_n(ptr: int, data: Buffer, size: int, reverse: bool = False):
    """@brief Write data from a Python buffer into recompiled memory.
    
    This function accepts not just `bytes` and `bytearray` objects, but any Python type that implements the Python 
    buffer protocol, such as arrays from Python's `array` module.
    
    If `size` is greater than the length of `data`, then writing will be truncated to fit size.
    If `size` is less than the length of `data`, then the remainder of the recompiled memory region will be untouched.

    @param ptr The mod memory address to write to. 
    @param data A Python `bytes` object to write to mod memory.
    @param size The maximum size of the memory to write.
    @param reverse If True, mirrors the memory being read. Defaults to False
    @throws RuntimeError if called from a thread not created by N64Recompiled.
    """
    _recomp_mem_byteswapped.write_buffer_n(ptr, data, size, reverse)
    
## @}
## @}