#include <array>
#include <cstdint>

using namespace std;
/*
number of updates - quadruples
array<uint_8, 64*4*ncolls>
*/
using ShmVoxelFrame = array<uint8_t, 64*4*2000>;

struct alignas(64) Header {
    uint32_t signature; //4
    uint16_t version; //2
    uint8_t reserved[58];
};

struct ShmLayout {
    Header header;
    ShmVoxelFrame data;
};

ShmLayout* initShm(const Header header, const char* name); //reader opens shm first, sets header, returns base ptr
ShmLayout* openShm(const char* name); //writer opens shm and returns base ptr
ShmLayout& readShm(const ShmLayout* basePtr);
void writeShm(ShmLayout* basePtr, const ShmVoxelFrame& shmVoxelFrame);
