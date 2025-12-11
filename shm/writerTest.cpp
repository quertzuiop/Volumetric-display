#include <shm.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    ShmLayout* ptr = openShm("testshm");
    int i = 0;
    while (++i) {
        ptr->data[1] = i;
        printf("wrote %d\n", i);
        sleep(2);
    }
}