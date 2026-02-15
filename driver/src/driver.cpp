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

    printf("initializing Shared memory\n");
    // volatile ShmLayout *shmPointer = openShm("vdshm");
    const Header header = {
        .signature = 0xB0B,
        .version = 1
    };
    volatile ShmLayout *shmPointer = initShm(header, "vdshm");


    printf("a\n");
    volatile ShmVoxelFrame& frame = shmPointer->data;

    auto startTime = Time::now();
    int frameNum = 0;

    //wait for speed regulator
    printf("a\n");
    while (shmPointer->nextFrameDuration == 0 && usePhotointerrupterFps) {}

    int64_t lastFrameStart = 0;
    printf("a\n");

    while (true) {

        // printf("frame start time: %lld\n", shmPointer->nextFrameStart);
        // printf("last frame start time: %lld\n", shmPointer->nextFrameDuration);

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

        // if (frameNum%24==0) {
        //     printf("Frame %d\n", frameNum);
        // }
        printf("Frame %d\n", frameNum);
        long frameSum = 0;
        for (int i = 0; i < 2000; i++) {
            const ShmVoxelSlice& slice = (const_cast<ShmVoxelFrame&>(frame))[i];
            //265.25
            //192.651
            auto tfdtwav = (nextFrameDuration/2000 * (i+1) + nextFrameStart);
            auto targetSliceEndTime = (nextFrameDuration/2000 * (i+1) + nextFrameStart);
            // printf("frame duration: %lld ns\n", frameDurationNs.count());
            // printf("slice duration: %lld ns\n", (frameDurationNs/2000 * (i+1)).count());
            // printf("waiting until: %lld ms\n", chrono::time_point_cast<chrono::milliseconds>(targetSliceEndTime).time_since_epoch().count());

            auto index1 = 31-static_cast<int> (slice.index1);
            auto index2 = 31-static_cast<int> (slice.index2);

            //printf("\nindex: %d\n", index1);
            addressInterface1.setAddress(index1);
            addressInterface2.setAddress(index2);

            for(int j = 0; j < 64; j++) {
                // if (slice.data[i] != 0) {
                //     printf("%d ", slice.data[i]);
                // }
                frameSum += slice.data[0+j] + slice.data[64+j] + slice.data[128+j] + slice.data[192+j];
                // colorInterface.pushColor(slice.data[0+j], slice.data[64+j], slice.data[128+j], slice.data[192+j]);
                //colorInterface.pushColor(slice.data[0+j], slice.data[64+j], slice.data[128+j], slice.data[192+j]);
                colorInterface.pushColor(slice.data[63-j], slice.data[127-j], slice.data[191-j], slice.data[255-j]);


            }
            outputInterface.showUntil(targetSliceEndTime);
            // usleep(10000);
        }
        printf("frame sum: %ld\n", frameSum);
        frameNum++;
    }
}