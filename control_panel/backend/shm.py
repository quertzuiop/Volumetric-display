from multiprocessing import shared_memory
import ctypes

SHM_SIGNATURE = 0xB0B
SHM_VERSION = 1

class Header(ctypes.Structure):
    _fields_ = [
        ("signature", ctypes.c_uint32),    # 4 bytes
        ("version", ctypes.c_uint16),      # 2 bytes
        ("reserved", ctypes.c_uint8 * 58)  # 58 bytes (Padding to reach 64 bytes)
    ]
    # Total size: 64 bytes (matches alignas(64))

class ShmVoxelSlice(ctypes.Structure):
    _fields_ = [
        ("index1", ctypes.c_uint8),        # 1 byte
        ("index2", ctypes.c_uint8),        # 1 byte
        ("data", ctypes.c_uint8 * 256)     # 64 * 4 = 256 bytes
    ]
    # Total size: 258 bytes

ShmVoxelFrame = ShmVoxelSlice * 2000 

class ShmLayout(ctypes.Structure):
    _fields_ = [
        ("header", Header),                     # Offset 0
        ("nextFrameStart", ctypes.c_int64),     # Offset 64
        ("nextFrameDuration", ctypes.c_int64),  # Offset 72
        ("keyboardState", ctypes.c_char * 8),   # Offset 80
        ("data", ShmVoxelFrame)                 # Offset 88
    ]
    
class Shm:
    def __init__(self, name):
        self.name = name
        self.size = ctypes.sizeof(ShmLayout)
        self.shm = None
        self.layout = None
        
    def create(self):
        try:
            self.shm = shared_memory.SharedMemory(name=self.name, create=True, size = self.size)
        
        except FileExistsError:
            temp_shm = shared_memory.SharedMemory(name=self.name, create=False, size = self.size)
            temp_shm.unlink()
            temp_shm.close()
            
            self.shm = shared_memory.SharedMemory(name=self.name, create=True, size = self.size)
        
        self.layout = ShmLayout.from_buffer(self.shm.buf)

        self.layout.header.signature = SHM_SIGNATURE
        self.layout.header.version = SHM_VERSION
    
    def write_keys(self, key_strokes):
        ctypes.memset(ctypes.addressof(self.layout.keyboardState), 0, 8)
        for i, key in enumerate(key_strokes[:8]):
            if key.key_code < 0: continue
            self.layout.keyboardState[i] = key.key_code
    
    def close(self):
        if self.shm == None:
            print("shm not created yet")
            return
        self.shm.close()
