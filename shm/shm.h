#pragma once

#include <array>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include "../renderer/include/types.h"

using namespace std;

struct ShmVoxelSlice {
    uint8_t index1;
    uint8_t index2;
    array<uint8_t, 64*4> data;
};

struct alignas(64) Header {
    uint32_t signature; //4
    uint16_t version;   //2
    uint32_t sliceCount;
    uint8_t reserved[52];
};

struct ShmLayout {
    Header header;
    int64_t nextFrameStart;
    int64_t nextFrameDuration;
    KeyboardState keyboardState;

    ShmVoxelSlice data[]; 
};

ShmLayout* initShm(const Header header, const char* name, int colCount);

ShmLayout* openShm(const char* name);