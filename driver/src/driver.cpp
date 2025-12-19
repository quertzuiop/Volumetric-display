#include <time.h>
#include <chrono>
#include "shm.h"
#include "displayControl.h"
#include <unistd.h>
#include<cstring>
#include<iostream>


const int fps = 24;
const float sliceDurationUs = 1000000/fps/2000;

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
    ShmLayout *shmPointer = initShm(header, "vdshm");
    ShmVoxelFrame& frame = shmPointer->data;

    auto startTime = Time::now();
    int frameNum = 0;
    while (true) {
        auto frameStartTime = Time::now();
        double frameDurationSeconds = 1.0 / 30.; 

        auto frameDurationNs = chrono::duration_cast<chrono::nanoseconds>(
            chrono::duration<double>(frameDurationSeconds)
        );
        if (frameNum%24==0) {
            printf("Frame %d\n", frameNum);
        }
        for (int i = 0; i < frame.size(); i++) {
            const ShmVoxelSlice& slice = frame[i];

            auto targetSliceEndTime = (frameDurationNs/2000 * (i+1) + frameStartTime);
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
                colorGroup.pushColor(slice.data[i], slice.data[64+i]);
            }
            outputInterface.showUntil(targetSliceEndTime);
            // usleep(10000);
        }
        frameNum++;
    }
}