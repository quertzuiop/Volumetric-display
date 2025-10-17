#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <list>
#include <chrono>
#include <sched.h>
#include <pthread.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "displayControl.h"

using namespace std;
using namespace std::chrono;

#define BCM2711_PERI_BASE        0xFE000000
#define GPIO_REGISTER_OFFSET         0x200000
#define GPIO_BASE (BCM2711_PERI_BASE + GPIO_REGISTER_OFFSET)

const int PAGE_SIZE = 4*1024;
const int BLOCK_SIZE = 4*1024;

int mem_fd;
void *gpio_map;
volatile unsigned *gpio;

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

int addressPins[] = {22, 23, 24, 25, 15};
list<int> initializedPins;

void setAddress(int address) {
    for (int i = 0; i < 5; i++) {
        if ((address>>i)%2==1) {
            GPIO_SET = (1<<addressPins[i]);
        }
        else {
            GPIO_CLR = (1<<addressPins[i]);
        }
    }
}
void cleanup() {
    for (int pin: initializedPins) {
        GPIO_CLR = (1<<pin);
        usleep(100);
    }
    munmap(gpio_map, BLOCK_SIZE);
}
void pinInit(int pin, bool initialHigh=false) {
    if (pin > 27) {cleanup(); exit(-1);}
    INP_GPIO(pin);
    OUT_GPIO(pin);
    if (initialHigh) {
        GPIO_SET = (1<<pin);
    } else {
        GPIO_CLR = (1<<pin);
    }
    initializedPins.push_front(pin);
    usleep(100);
}

void setup_io();

static inline void spin_asm() {
    asm("");
}

void busy_wait_nanos(long nanos) {
    auto t1 = steady_clock::now();
	while (duration_cast<nanoseconds>(steady_clock::now() - t1).count() < nanos - 100) {
		spin_asm();
	}
}

void tiny_wait(int n) {
    volatile int sink=0;
    for (int j = 0; j<n; ++j) {
        sink += j;
    }
}

ColorGroupInterface::ColorGroupInterface (int colorPins[6], int clockPin) {
    for (int i = 0; i < 6; i++) {
        pinNums[i] = colorPins[i];
    }
    clockPinNum=clockPin;
    for (int pin: pinNums) {
        pinInit(pin);
    }
    pinInit(clockPinNum);
}

void ColorGroupInterface::pushColor(int c1, int c2) {
    int regVal = ((c1 & 1)<<pinNums[0]) | ((c1>>1 & 1)<<pinNums[1]) | ((c1>>2 & 1)<<pinNums[2])
                | ((c2 & 1)<<pinNums[3]) | ((c2>>1 & 1)<<pinNums[4]) | ((c2>>2 & 1)<<pinNums[5]);
    GPIO_SET = regVal;
    GPIO_SET = (1<<clockPinNum);
    tiny_wait(15);
    GPIO_CLR = regVal | (1<<clockPinNum);
    tiny_wait(5);
}

AddressInterface::AddressInterface(int pins[5]) {
    for (int i = 0; i < 5; i++) {
        addressPins[i] = pins[i];
        pinInit(addressPins[i]);
    }
}
void AddressInterface::setAddress(int address) {
    for (int i = 0; i < 5; i++) {
        if ((address>>i)%2==1) {
            GPIO_SET = (1<<addressPins[i]);
        }
        else {
            GPIO_CLR = (1<<addressPins[i]);
        }
    }
}

OutputInterface::OutputInterface(int latchPin_, int oePin_) {
    latchPin = latchPin_;
    oePin = oePin_;
    pinInit(latchPin);
    pinInit(oePin, true);
}

void OutputInterface::show() {
    GPIO_SET = (1<<oePin);
    GPIO_SET = (1<<latchPin);
    tiny_wait(10);
    GPIO_CLR = (1<<latchPin);
    GPIO_CLR = (1<<oePin);
}
void OutputInterface::latch() {
    GPIO_SET = (1<<latchPin);
    tiny_wait(10);
    GPIO_CLR = (1<<latchPin);
    tiny_wait(5);
}

void OutputInterface::enableOutput(bool enable) {
    if (enable) {
        GPIO_CLR = (1<<oePin);
    } else {
        GPIO_SET = (1<<oePin);
    }
}

class PinInterface {
    public:
        int pinNum;
        bool value;
        void on() {
            GPIO_SET = (1<<pinNum);
            value=true;
        }
        void off() {
            GPIO_CLR = (1<<pinNum);
            value=false;
        }
    PinInterface(int pinNum_) {
        if (pinNum <= 27  and pinNum >= 0) {
            pinNum=pinNum_;
            pinInit(pinNum_);
        } else {
            cout << "failed to initialize pin " << pinNum <<endl;
        }
    }
};



void setup_io() {
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }
    gpio_map = mmap(
        NULL,             //Any address in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writing to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
    );
    close(mem_fd);
    
    if (gpio_map == MAP_FAILED) {
      perror("mmap");//errno also set!
      exit(-1);
   }
   cout << "GPIO mapped at address " << gpio_map << endl;
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
}
