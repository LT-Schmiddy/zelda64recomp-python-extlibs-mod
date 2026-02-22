import _recomp_mem_managed

from .. import utils

def read_u8(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_u8(ptr)
    
def write_u8(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_u8(ptr, value)

def read_u16(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_u16(ptr)
    
def write_u16(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_u16(ptr, value)

def read_u32(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_u32(ptr)
    
def write_u32(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_u32(ptr, value)

def read_u64(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_u64(ptr)
    
def write_u64(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_u64(ptr, value)

# Signed Ints:
def read_s8(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_s8(ptr)
    
def write_s8(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_s8(ptr, value)

def read_s16(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_s16(ptr)
    
def write_s16(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_s16(ptr, value)

def read_s32(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_s32(ptr)
    
def write_s32(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_s32(ptr, value)

def read_s64(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_s64(ptr)
    
def write_s64(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_s64(ptr, value)

# Floats:
def read_f32(ptr: int) -> float:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_f32(ptr)
    
def write_f32(ptr: int, value: float):
    utils.assert_main_thread()
    _recomp_mem_managed.write_f32(ptr, value)


def read_f64(ptr: int) -> float:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_f64(ptr)
    
def write_f64(ptr: int, value: float):
    utils.assert_main_thread()
    _recomp_mem_managed.write_f64(ptr, value)

# Text:
def read_char(ptr: int) -> str:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_char(ptr)
    
def write_char(ptr: int, char: str):
    utils.assert_main_thread()
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_managed.write_char(ptr, char)

def read_byte_char(ptr: int) -> bytes:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_byte_char(ptr)
    
def write_byte_char(ptr: int, char: bytes):
    utils.assert_main_thread()
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem_managed.write_byte_char(ptr, char)

def read_str(ptr: int) -> str:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_str(ptr)

def read_str_n(ptr: int, size: int) -> str:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_str_n(ptr, size)

def write_str_n(ptr: int, data: str, size: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_str_n(ptr, data, size)

def read_byte_str(ptr: int) -> bytes:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_byte_str(ptr)

def read_byte_str_n(ptr: int, size: int) -> bytes:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_byte_str_n(ptr, size)

def write_byte_str_n(ptr: int, data: str, size: int):
    utils.assert_main_thread()
    _recomp_mem_managed.write_byte_str_n(ptr, data, size)

# Memory
def read_bytes_n(ptr: int, size: int, reverse: bool = False) -> bytes:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_bytes_n(ptr, size, reverse)
    
def read_bytearray_n(ptr: int, size: int, reverse: bool = False) -> bytearray:
    utils.assert_main_thread()
    return _recomp_mem_managed.read_bytearray_n(ptr, size, reverse)
    
def write_buffer_n(ptr: int, data: bytearray, size: int, reverse: bool = False):
    utils.assert_main_thread()
    _recomp_mem_managed.write_buffer_n(ptr, data, size, reverse)
    
