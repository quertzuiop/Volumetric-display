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

static void busy_wait_nanos(long nanos) {
    auto t1 = steady_clock::now();
	while (duration_cast<nanoseconds>(steady_clock::now() - t1).count() < nanos - 100) {
		spin_asm();
	}
}

static void tiny_wait(int n) {
    volatile int sink=0;
    for (int j = 0; j<n; ++j) {
        sink += j;
    }
}
class ColorGroupInterface { //controls color pins and clock 
    public:
        uint8_t pinNums[6];
        uint8_t clockPinNum;
        inline void pushColor(uint8_t c1, uint8_t c2) {
            int regVal = ((c1 & 1)<<pinNums[0]) | ((c1>>1 & 1)<<pinNums[1]) | ((c1>>2 & 1)<<pinNums[2])
                        | ((c2 & 1)<<pinNums[3]) | ((c2>>1 & 1)<<pinNums[4]) | ((c2>>2 & 1)<<pinNums[5]);
            GPIO_SET = regVal;
            GPIO_SET = (1<<clockPinNum);
            tiny_wait(10);
            GPIO_CLR = regVal | (1<<clockPinNum);
            tiny_wait(5);
        }
    ColorGroupInterface (int ColorPins[6], int clockPin) {
        for (int i = 0; i < 6; i++) {
            pinNums[i] = ColorPins[i];
        }
        clockPinNum=clockPin;
        for (int pin: pinNums) {
            pinInit(pin);
        }
        pinInit(clockPinNum);
    }
};
class AddressInterface {
    public:
        int addressPins[5];
        inline void setAddress(int address) {
            for (int i = 0; i < 5; i++) {
                if ((address>>i)%2==1) {
                    GPIO_SET = (1<<addressPins[i]);
                }
                else {
                    GPIO_CLR = (1<<addressPins[i]);
                }
            }
        }
    AddressInterface(int pins[5]) {
        for (int i = 0; i < 5; i++) {
            addressPins[i] = pins[i];
            pinInit(addressPins[i]);
        }
    }
};
class OutputInterface {
    public:
        int latchPin, oePin;
        inline void show() {
            GPIO_SET = (1<<latchPin);
            tiny_wait(10);
            GPIO_CLR = (1<<latchPin);
            tiny_wait(5);
            GPIO_CLR = (1<<oePin);
            busy_wait_nanos(2000); //visible time
            GPIO_SET = (1<<oePin);
        }
    OutputInterface(int latchPin_, int oePin_) {
        latchPin = latchPin_;
        oePin = oePin_;
        pinInit(latchPin);
        pinInit(oePin, true);
    }
};
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
int main(int argc, char **argv)
{   
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(3, &cpus);
    if (sched_setaffinity(0, sizeof(cpus), &cpus) != 0) {
        cerr << "sched_setaffinity failed: " << strerror(errno) << " (continuing)\n";
    }

    cout << "Raspberry Pi GPIO test program" << endl;
    setup_io();
    cout << "GPIO setup done: " << (void*)gpio << endl;

    ColorGroupInterface colorGroup(new int[6]{11, 27, 7, 8, 9, 10}, 17);
    AddressInterface addressInterface(new int[5]{22, 23, 24, 25, 15});
    OutputInterface outputInterface(4, 18);

    const int frames = 20000;

    // Main loop, measure one timestamp per frame (in microseconds)
    auto frame_start = steady_clock::now();
    for (int i = 0; i < frames; i++) {
        for (int a = 0; a < 32; a++) {
            auto frame_start = steady_clock::now();
            addressInterface.setAddress(a);
            for (int px = 0; px < 64; px++) {
                if (px <32) {
                    colorGroup.pushColor(0b100, 0b000);
                } else {
                    colorGroup.pushColor(0b000, 0b001);
                }
            }
            outputInterface.show();
        }
       
    }
    auto frame_end = steady_clock::now();
    auto fps = (double)frames / ((double)duration_cast<milliseconds>(frame_end - frame_start).count() / 1000.0);
    cout << "fps: " << fps << endl;
    cout << "displaycols " << (fps / 64 * 24) <<endl;;
    cleanup();
    cout << "GPIO unmap done." << endl;
    return 0;
}


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
