#pragma once

#include <array>
#include <cstdint>
#include <chrono>
#include "../renderer/include/types.h"

using namespace std;
/*
number of updates - quadruples
array<uint_8, 64*4*ncolls>
*/
// (rowIndex, array<uint, 64*2> x 2)

/*
PROTOCOL
(launch control_panel)
control_panel - init shm
(launch speed regulator)
speed regulator spins up, starts writing speeds to shm
(launch driver)
driver starts displaying shm content
(launch app)
app starts writing to shm
*/
struct ShmVoxelSlice {
    uint8_t index1;
    uint8_t index2;
    array<uint8_t, 64*4> data;
};

using ShmVoxelFrame = array<ShmVoxelSlice, 2000>;

struct alignas(64) Header {
    uint32_t signature; //4
    uint16_t version; //2
    uint8_t reserved[58];
};

struct ShmLayout {
    Header header;
    int64_t nextFrameStart;
    int64_t nextFrameDuration = 0;
    KeyboardState keyboardState;
    ShmVoxelFrame data;
};

ShmLayout* initShm(const Header header, const char* name); //reader opens shm first, sets header, returns base ptr
ShmLayout* openShm(const char* name); //writer opens shm and returns base ptr

ShmLayout& readShm(const ShmLayout* basePtr);
void writeShm(ShmLayout* basePtr, const ShmVoxelFrame& shmVoxelFrame);
