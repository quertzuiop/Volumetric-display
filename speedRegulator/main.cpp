#include "shm.h"
#include <chrono>
#include <wiringPi.h>

using namespace std;
const double targetFps = 24.;

void waitForFall() {
    while (digitalRead(1) == HIGH) {}
}

int main() {
    volatile ShmLayout* shmPointer = openShm("vdshm");
    wiringPiSetupGpio();
    pinMode(1, INPUT);

    int64_t lastFrameStart;
    int frameNum = 0;
    printf("speed regulator setup succesfully\n");
    while (true) {
        auto currentTime = chrono::time_point_cast<chrono::nanoseconds>(chrono::steady_clock::now()).time_since_epoch().count();

        if (frameNum >= 1) {
            auto frameDuration =  chrono::steady_clock::now().time_since_epoch().count() - lastFrameStart;
            shmPointer->nextFrameStart = currentTime;
            shmPointer->nextFrameDuration = frameDuration;
        }
        lastFrameStart = currentTime;
        while (true) {
            while (digitalRead(1) == HIGH) {}
            int count = 0; 
            while (digitalRead(1) == LOW && count < 100) {
                count++;
            }
            if (count == 100) { break; }
        }

        while (true) {
            while (digitalRead(1) == LOW) {}
            int count = 0; 
            while (digitalRead(1) == HIGH && count < 100) {
                count++;
            }
            if (count == 100) { break; }
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