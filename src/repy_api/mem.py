import _recomp_mem

from . import utils

def read_u8(ptr: int) -> int:
    utils.assert_main_thread()
    return _recomp_mem.read_u8(ptr)
    
def write_u8(ptr: int, value: int):
    utils.assert_main_thread()
    _recomp_mem.write_u8(ptr, value)

def read_u16(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_u16(ptr)
    
def write_u16(ptr: int, value: int):
    utils.assert_main_thread()

    if not isinstance(value, int):
        value = int(value)
        
    _recomp_mem.write_u16(ptr, value)

def read_u32(ptr: int) -> int:
    utils.assert_main_thread()
        
    return _recomp_mem.read_u32(ptr)
    
def write_u32(ptr: int, value: int):
    utils.assert_main_thread()
        
    _recomp_mem.write_u32(ptr, value)

def read_u64(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_u64(ptr)
    
def write_u64(ptr: int, value: int):
    utils.assert_main_thread()
    
    _recomp_mem.write_u64(ptr, value)

# Signed Ints:
def read_s8(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_s8(ptr)
    
def write_s8(ptr: int, value: int):
    utils.assert_main_thread()
        
    _recomp_mem.write_s8(ptr, value)

def read_s16(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_s16(ptr)
    
def write_s16(ptr: int, value: int):
    utils.assert_main_thread()

    if not isinstance(value, int):
        value = int(value)
        
    _recomp_mem.write_s16(ptr, value)

def read_s32(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_s32(ptr)
    
def write_s32(ptr: int, value: int):
    utils.assert_main_thread()
    
    _recomp_mem.write_s32(ptr, value)

def read_s64(ptr: int) -> int:
    utils.assert_main_thread()
    
    return _recomp_mem.read_s64(ptr)
    
def write_s64(ptr: int, value: int):
    utils.assert_main_thread()
        
    _recomp_mem.write_s64(ptr, value)

# Floats:
def read_f32(ptr: int) -> float:
    utils.assert_main_thread()
        
    return _recomp_mem.read_f32(ptr)
    
def write_f32(ptr: int, value: float):
    utils.assert_main_thread()
    
    _recomp_mem.write_f32(ptr, value)


def read_f64(ptr: int) -> float:
    utils.assert_main_thread()
        
    return _recomp_mem.read_f64(ptr)
    
def write_f64(ptr: int, value: float):
    utils.assert_main_thread()
    
    _recomp_mem.write_f64(ptr, value)

# Text:
def read_char(ptr: int) -> str:
    utils.assert_main_thread()
        
    return _recomp_mem.read_char(ptr)
    
def write_char(ptr: int, char: str):
    utils.assert_main_thread()
    
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem.write_char(ptr, char)

def read_byte_char(ptr: int) -> bytes:
    utils.assert_main_thread()
        
    return _recomp_mem.read_byte_char(ptr)
    
def write_byte_char(ptr: int, char: bytes):
    utils.assert_main_thread()
        
    if len(char) > 1:
        raise ValueError(f"{char=} has a length greater than 1")
    
    _recomp_mem.write_byte_char(ptr, char)

def read_str(ptr: int) -> str:
    utils.assert_main_thread()

    return _recomp_mem.read_str(ptr)

def read_str_n(ptr: int, size: int) -> str:
    utils.assert_main_thread()
        
    return _recomp_mem.read_str_n(ptr, size)

def write_str_n(ptr: int, data: str, size: int):
    utils.assert_main_thread()
        
    _recomp_mem.write_str_n(ptr, data, size)

def read_byte_str(ptr: int) -> bytes:
    utils.assert_main_thread()
        
    return _recomp_mem.read_byte_str(ptr)

def read_byte_str_n(ptr: int, size: int) -> bytes:
    utils.assert_main_thread()
        
    return _recomp_mem.read_byte_str_n(ptr, size)

def write_byte_str_n(ptr: int, data: str, size: int):
    utils.assert_main_thread()
        
    _recomp_mem.write_byte_str_n(ptr, data, size)

# Memory
def read_bytes_n(ptr: int, size: int) -> bytes:
    utils.assert_main_thread()

    return _recomp_mem.read_bytes_n(ptr, size)
    
def read_bytearray_n(ptr: int, size: int) -> bytearray:
    utils.assert_main_thread()
        
    return _recomp_mem.read_bytearray_n(ptr, size)
    
def write_buffer_n(ptr: int, data: bytearray, size: int):
    utils.assert_main_thread()
    
    _recomp_mem.write_buffer_n(ptr, data, size)
    
