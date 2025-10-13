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

void pushColor(bool r, bool g, bool b) {

}

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
    for (int a = 0; a < 5; a++) {
        pinInit(addressPins[a]);
    }
    pinInit(11); //r1
    pinInit(4); //latch
    pinInit(17); //clock
    pinInit(18, true); //oe

    pinInit(8); //other color pins
    pinInit(27);
    pinInit(9);
    pinInit(7);
    pinInit(10);

    const int frames = 10000;

    // Main loop, measure one timestamp per frame (in microseconds)
    auto frame_start = steady_clock::now();
    for (int i = 0; i < frames; i++) {

        for (int a = 0; a < 32; a++) {
            auto frame_start = steady_clock::now();
            setAddress(a);
            for (int px = 0; px < 64; px++) {
                if (px <32) {
                    GPIO_SET = (1<<11);
                } else {
                    GPIO_SET = (1<<9);
                }
                GPIO_SET = (1<<17); //clk on
                busy_wait_nanos(50);
                GPIO_CLR = (1<<11) | (1<<9) | (1<<17); //r1 and clk off
                busy_wait_nanos(50);
                GPIO_SET = (1<<4); //latch on
                busy_wait_nanos(0);
                GPIO_CLR = (1<<4); //latch off, oe off (on)
            }
            //busy_wait_nanos(0);
            GPIO_CLR = (1<<18);
            busy_wait_nanos(500);
            GPIO_SET = (1<<18); //oe on (off)
            //;
            //auto dur_us = (uint32_t)duration_cast<nanoseconds>(frame_end - frame_start).count();
            //if (dur_us > 100000) {cout<<"konec"<<endl;}
            //samples_us.push_back(dur_us);
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
