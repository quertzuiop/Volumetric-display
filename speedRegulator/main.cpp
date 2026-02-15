#include "shm.h"
#include <chrono>
#include <wiringPi.h>

using namespace std;
using namespace chrono_literals;
const double targetFps = 24.;
const int dataPin = 16;
const auto minOnTime = 10ms;
const float minFps = 0.5;

int main() {
    volatile ShmLayout* shmPointer = openShm("vdshm");
    wiringPiSetupGpio();
    pinMode(dataPin, INPUT);

    int64_t lastFrameStart;
    int frameNum = 0;
    printf("speed regulator setup succesfully\n");
    while (true) {
        auto currentTime = chrono::time_point_cast<chrono::nanoseconds>(chrono::steady_clock::now()).time_since_epoch().count();

        if (frameNum >= 1) {
            auto frameDuration =  chrono::steady_clock::now().time_since_epoch().count() - lastFrameStart;
            float fps = 1000000000.0/frameDuration;
            printf("frame duration: %ld, fps: %f\n", frameDuration, 1000000000.0/frameDuration);
            if (fps > minFps) {
                shmPointer->nextFrameStart = currentTime;
                shmPointer->nextFrameDuration = frameDuration;
            }
        }
        lastFrameStart = currentTime;
        while (true) {
            while (digitalRead(dataPin) == HIGH) {}
            auto start = chrono::steady_clock::now();
            bool passed = false;
            while (digitalRead(dataPin) == LOW) {
                if (chrono::steady_clock::now() - start > minOnTime) {
                    passed = true;
                    break;
                } 
            }
            if (passed) { break; }
        }

        while (true) {
            while (digitalRead(dataPin) == LOW) {}
            auto start = chrono::steady_clock::now();
            bool passed = false;
            while (digitalRead(dataPin) == HIGH) {
                if (chrono::steady_clock::now() - start > minOnTime) {
                    passed = true;
                    break;
                } 
            }
            if (passed) { break; }
        }

        frameNum++;
    }
    // auto start = chrono::steady_clock::now();
    // auto a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    // for (int i = 0; i < 48000; i++) {
    //     a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    // }
    // auto end = chrono::steady_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    // printf("Total time for 10000 calls: %lld microseconds\n", duration);
}