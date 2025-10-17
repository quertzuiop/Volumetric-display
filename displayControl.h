#include <cstdint>
#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H
class ColorGroupInterface {
    public:
        int pinNums[6];
        int clockPinNum;
        void pushColor(int c1, int c2);
    ColorGroupInterface (int ColorPins[6], int clockPin);
};
class AddressInterface {
    public:
        int addressPins[5];
        void setAddress(int address);
    AddressInterface(int pins[5]);
};
class OutputInterface {
    public:
        int latchPin, oePin;
        void show();
        void enableOutput(bool enable);
        void latch();
    OutputInterface(int latchPin_, int oePin_);
};
void busy_wait_nanos(long nanos);
void tiny_wait(int n);
void cleanup();
void setup_io();
#endif