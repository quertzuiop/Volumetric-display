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
    // array<int, 6> colorPins1 = {11, 27, 7, 8, 9, 10};

    // int clockPin = 17;
    // ColorInterface colorInterface(colorPins1, clockPin);

    // int addressPins[5] = {22, 23, 24, 25, 15};
    // AddressInterface addressInterface(addressPins);

    // int latchPin = 4;
    // int oePin = 18;
    // OutputInterface outputInterface(latchPin, oePin);

    
    array<int, 6> colorPins1 = {11, 27, 7, 8, 9, 10};
    array<int, 6> colorPins2 = {12, 5, 6, 19, 13, 20};
    int clockPin = 17;
    ColorInterface colorInterface(colorPins1, colorPins2, clockPin);

    array<int, 5> addressPins1 = {22, 23, 24, 25, 15};
    array<int, 5> addressPins2 = {1, 1, 1, 1, 1};
    AddressInterface addressInterface1(addressPins1);
    AddressInterface addressInterface2(addressPins2);

    int latchPin = 4;
    int oePin = 18;
    OutputInterface outputInterface(latchPin, oePin);


    // Test the interfaces
    outputInterface.enableOutput(true);
    colorInterface.pushColor(0b111, 0, 0, 0);
    const int frames = 20000;

    // Main loop, measure one timestamp per frame (in microseconds)
    auto frame_start = steady_clock::now();
    for (int i = 0; i < frames; i++) {
        for (int a = 0; a < 32; a++) {
            auto frame_start = steady_clock::now();
            addressInterface1.setAddress(a);
            addressInterface2.setAddress(a);
            for (int px = 0; px < 64; px++) {
                if (px <32) {
                    colorInterface.pushColor((a==5) ? 0b100 : 0b000, 0b000, 0, 0);
                } else {
                    colorInterface.pushColor(0b000, 0b001*px%3, 0b001*px%4, 0);
                }
            }
            outputInterface.showUntil(chrono::time_point_cast<chrono::nanoseconds>(chrono::steady_clock::now()).time_since_epoch().count() + 100000);
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