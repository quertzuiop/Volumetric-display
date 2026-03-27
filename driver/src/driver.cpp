#include <time.h>
#include <chrono>
#include "shm.h"
#include "displayControl.h"
#include <unistd.h>
#include<cstring>
#include<iostream>
#include <cassert>    // Required for assert
#include <typeinfo>   // Required for typeid
#include <vector>
#include <string>

using namespace std;
using namespace chrono_literals;
using Time = chrono::steady_clock;

const float gammaCorrectionRatio = 0.3;

template<typename T>
concept optionType = same_as<T, int> ||
                     same_as<T, bool>;

//passing bool returns true if the flag is present, shouldnt have true or false after
template<optionType T>
T getOption(string_view argname, int argc, char* argv[]) {
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i] == argname) {
            if constexpr(same_as<T, int>) {
                if (i + 1 < argc) {
                    return atoi(argv[i+1]);
                }
            } else if constexpr(same_as<T, bool>) {
                return true;
            }
        }
    }
    if constexpr(same_as<T, bool>) {
        return false; //dont expect argument
    }

    throw invalid_argument("argument not found");
}


int main(int argc, char* argv[]) {
    bool usePhotointerrupterFps = true; 
    int  fps = 0;

    try {
        fps = getOption<int>("-fps", argc, argv);
        usePhotointerrupterFps = false;
    } catch (invalid_argument& e) {
        printf("fps value cannot be parsed\n");
        // usePhotointerrupterFps = false;
        // fps = 10;
    }

    if (usePhotointerrupterFps) {
        printf("Using photinterrupter for fps\n");
    } else {
        printf("Fps set to: %d\n", fps);
    }

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(1, &cpus);
    if (sched_setaffinity(0, sizeof(cpus), &cpus) != 0) {
        cerr << "sched_setaffinity failed: " << strerror(errno) << " (continuing)\n";
    }
    setup_io();

    array<int, 6> colorPins1 = {11, 27, 7, 8, 9, 10};
    array<int, 6> colorPins2 = {12, 5, 6, 19, 13, 20};
    int clockPin = 17;
    ColorInterface colorInterface(colorPins1, colorPins2, clockPin);

    array<int, 5> addressPins1 = {22, 23, 24, 25, 15};
    array<int, 5> addressPins2 = {2, 3, 21, 26, 14};
    AddressInterface addressInterface1(addressPins1);
    AddressInterface addressInterface2(addressPins2);

    int latchPin = 4;
    int oePin = 18;
    OutputInterface outputInterface(latchPin, oePin);

    printf("Opening Shared memory\n");
    volatile ShmLayout *shmPointer = openShm("vdshm");

    volatile auto& frame = shmPointer->data;

    auto sliceCount = shmPointer->header.sliceCount;
    printf("[driver] slice count: %d\n", sliceCount);

    auto useTwoBitColor = shmPointer->header.twoBitColor;

    auto startTime = Time::now();
    int frameNum = 0;

    //wait for speed regulator
    while (shmPointer->nextFrameDuration == 0 && usePhotointerrupterFps) {}
    printf("first frame duration info received");
    int64_t lastFrameStart = 0;

    while (true) {
        while (lastFrameStart == shmPointer->nextFrameStart && usePhotointerrupterFps) {} //if new frame hasnt started (we are ahead), wait 
        int64_t nextFrameStart;
        int64_t nextFrameDuration;
        if (usePhotointerrupterFps) {
            nextFrameStart = shmPointer->nextFrameStart;
            nextFrameDuration = shmPointer->nextFrameDuration;
        } else {
            nextFrameStart = chrono::time_point_cast<chrono::nanoseconds>(chrono::steady_clock::now()).time_since_epoch().count();
            nextFrameDuration = 1000000000/fps;
        }
        
        lastFrameStart = nextFrameStart;

        printf("Frame %d\n", frameNum);
        long frameSum = 0;
        if (not useTwoBitColor) {
            for (int i = 0; i < sliceCount; i++) {
                const ShmVoxelSlice& slice = const_cast<ShmVoxelSlice&>(shmPointer->data[i]);
                //265.25
                //192.651
                auto targetSliceEndTime = (nextFrameDuration/sliceCount * (i+1) + nextFrameStart);
    
                auto index1 = 31-static_cast<int> (slice.index1);
                auto index2 = 31-static_cast<int> (slice.index2);
    
                addressInterface1.setAddress(index1);
                addressInterface2.setAddress(index2);
    
                for(int j = 0; j < 64; j++) {
                    frameSum += slice.data[0+j] + slice.data[64+j] + slice.data[128+j] + slice.data[192+j];
                    colorInterface.pushColor(slice.data[63-j], slice.data[127-j], slice.data[191-j], slice.data[255-j], 0b10);
                }
                outputInterface.showUntil(targetSliceEndTime);
                // usleep(10000);
            }
        } else {
            for (int i = 0; i < sliceCount; i++) {
                const ShmVoxelSlice& slice = const_cast<ShmVoxelSlice&>(shmPointer->data[i]);
                
                auto index1 = 31-static_cast<int> (slice.index1);
                auto index2 = 31-static_cast<int> (slice.index2);
                
                addressInterface1.setAddress(index1);
                addressInterface2.setAddress(index2);

                auto time = std::chrono::steady_clock::now().time_since_epoch().count();

                auto targetSliceEndTime = (nextFrameDuration/sliceCount * (i+1) + nextFrameStart);

                auto targetFirstBitEndTime = time + gammaCorrectionRatio * (targetSliceEndTime - time);
                
                for(int j = 0; j < 64; j++) {
                    frameSum += slice.data[0+j] + slice.data[64+j] + slice.data[128+j] + slice.data[192+j];
                    colorInterface.pushColor(slice.data[63-j], slice.data[127-j], slice.data[191-j], slice.data[255-j], 0b10);    
                }
                outputInterface.showUntil(targetFirstBitEndTime);
                
                for(int j = 0; j < 64; j++) {
                    frameSum += slice.data[0+j] + slice.data[64+j] + slice.data[128+j] + slice.data[192+j];
                    colorInterface.pushColor(slice.data[63-j], slice.data[127-j], slice.data[191-j], slice.data[255-j], 0b01);    
                }
                outputInterface.showUntil(targetSliceEndTime);
            }
        }
        printf("frame sum: %ld\n", frameSum);
        frameNum++;
    }
}