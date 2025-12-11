#include <shm.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;
int main() {
    const Header header = {
        .signature = 0xB0B,
        .version = 1
    };
    ShmLayout* ptr = initShm(header, (const char*)"testshm");
    while (true) {
        ShmVoxelFrame& frame = ptr->data;
        printf("reading frame\n");
        for (uint8_t val : frame) {
            if (val != 0) {
                printf("%d\n", val);
            }
        }
        sleep(1);
    }
}