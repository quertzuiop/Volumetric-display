#include <iostream>
#include "displayControl.h"
#include <cstring>
#include <chrono>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

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

    // Test the interfaces
    outputInterface.enableOutput(true);
    colorGroup.pushColor(0b111, 0);
    const int frames = 20000;

    // Main loop, measure one timestamp per frame (in microseconds)
    auto frame_start = steady_clock::now();
    for (int i = 0; i < frames; i++) {
        for (int a = 0; a < 32; a++) {
            auto frame_start = steady_clock::now();
            addressInterface.setAddress(a);
            for (int px = 0; px < 64; px++) {
                if (px <32) {
                    colorGroup.pushColor((a==5) ? 0b100 : 0b000, 0b000);
                } else {
                    colorGroup.pushColor(0b000, 0b001*px%3);
                }
            }
            outputInterface.show();
            //usleep(10000);
        }
       
    }
    auto frame_end = steady_clock::now();
    auto fps = (double)frames / ((double)duration_cast<milliseconds>(frame_end - frame_start).count() / 1000.0);
    cout << "fps: " << fps << endl;
    cout << "displaycols " << (fps / 64 * 24) <<endl;;
    
    outputInterface.enableOutput(false);
    cleanup();
    return 0;
}