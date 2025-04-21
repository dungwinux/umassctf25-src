from ctypes import *
from pathlib import *

lib = CDLL(Path.cwd() / 'switch.dll')

p = create_string_buffer(0x100)
s = b'TEST_FLAG'
sp = create_string_buffer(s)
lib.special(p, sp, c_uint(len(s)))
print(repr(p.raw))
