#include <time.h>
#include <chrono>
#include "shm.h"
#include "displayControl.h"
#include <unistd.h>
#include<cstring>
#include<iostream>

using namespace std;
using namespace chrono_literals;
using Time = chrono::steady_clock;

int main() {

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(3, &cpus);
    if (sched_setaffinity(0, sizeof(cpus), &cpus) != 0) {
        cerr << "sched_setaffinity failed: " << strerror(errno) << " (continuing)\n";
    }
    setup_io();
    int colorPins[6] = {11, 27, 7, 8, 9, 10};
    int clockPin = 17;
    ColorGroupInterface colorGroup(colorPins, clockPin);

    int addressPins[5] = {22, 23, 24, 25, 15};
    AddressInterface addressInterface(addressPins);

    int latchPin = 4;
    int oePin = 18;
    OutputInterface outputInterface(latchPin, oePin);


    const Header header = {
        .signature = 0xB0B,
        .version = 1
    };
    volatile ShmLayout *shmPointer = initShm(header, "vdshm");
    volatile ShmVoxelFrame& frame = shmPointer->data;

    auto startTime = Time::now();
    int frameNum = 0;

    //wait for speed regulator
    while (shmPointer->nextFrameDuration == 0) {}
    int64_t lastFrameStart = 0;
    while (true) {

        // printf("frame start time: %lld\n", shmPointer->nextFrameStart);
        // printf("last frame start time: %lld\n", shmPointer->nextFrameDuration);

        while (lastFrameStart == shmPointer->nextFrameStart) {} //if new frame hasnt started (we are ahead), wait 
        
        auto nextFrameStart = shmPointer->nextFrameStart;
        auto nextFrameDuration = shmPointer->nextFrameDuration;
        
        lastFrameStart = nextFrameStart;

        // if (frameNum%24==0) {
        //     printf("Frame %d\n", frameNum);
        // }
        printf("Frame %d\n", frameNum);

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
            //printf("\nindex: %d\n", index1);
            addressInterface.setAddress(index1);
            for(int i = 0; i < 64; i++) {
                // if (slice.data[i] != 0) {
                //     printf("%d ", slice.data[i]);
                // }
                colorGroup.pushColor(slice.data[64+i], slice.data[64+i]);
            }
            outputInterface.showUntil(targetSliceEndTime);
            // usleep(10000);
        }
        frameNum++;
    }
}