#include<shm.h>
#include<iostream>
#include<sys/shm.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>



ShmLayout* initShm(const Header header, const char* name) {
    int g_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (g_fd == -1) {
        perror("initShm: shm_open failed");
        return nullptr;
    }
    if (ftruncate(g_fd, sizeof(ShmLayout)) == -1) {
        perror("initShm: ftruncate failed");
        return nullptr;
    }

    void* ptr = mmap(0, sizeof(ShmLayout), PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("initShm: mmap failed");
        return nullptr;
    }

    ShmLayout* g_shmPtr = static_cast<ShmLayout*>(ptr);

    g_shmPtr->header = header;

    return g_shmPtr;
}

ShmLayout* openShm(const char* name) {
    int g_fd = shm_open(name, O_RDWR, 0666);
    if (g_fd == -1) {
        if (errno == ENOENT) {
            cerr << "Driver not running (SHM not found)." << endl;
        } else {
            perror("openShm: shm_open failed");
        }
        return nullptr;
    }

    void* ptr = mmap(0, sizeof(ShmLayout), PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("openShm: mmap failed");
        return nullptr;
    }

    return static_cast<ShmLayout*>(ptr);
}
void writeShm(ShmLayout* basePtr, const ShmVoxelFrame& newFrame) {
    if (basePtr) {
        std::memcpy(&basePtr->data, &newFrame, sizeof(ShmVoxelFrame));
    }
}