#pragma once

#include <array>
#include <cstdint>
#include <chrono>

#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

class ColorInterface {
    public:
        std::array<int, 6> pinNums1;
        std::array<int, 6> pinNums2;
        int clockPinNum;
        void pushColor(int c11, int c12, int c21, int c22);
    ColorInterface (std::array<int, 6> ColorPins1, std::array<int, 6> ColorPins2, int clockPin);
};
class AddressInterface {
    public:
        std::array<int, 5> addressPins;
        void setAddress(int address);
    AddressInterface(std::array<int, 5> pins);
};
class OutputInterface {
    public:
        int latchPin, oePin;
        void showUntil(int64_t stopTime);
        void enableOutput(bool enable);
        void latch();
    OutputInterface(int latchPin_, int oePin_);
};
void busy_wait_nanos(long nanos);
void tiny_wait(int n);
void cleanup();
void setup_io();
#endif