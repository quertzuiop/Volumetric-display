#include<shm.h>
#include<iostream>
#include<sys/shm.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include <assert.h>



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
ShmLayout* initShm(const Header header, const char* name, int colCount) {
    int g_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (g_fd == -1) {
        perror("initShm: shm_open failed");
        return nullptr;
    }
    
    // Dynamically calculate required size
    size_t totalSize = sizeof(ShmLayout) + (colCount * sizeof(ShmVoxelSlice));
    
    if (ftruncate(g_fd, totalSize) == -1) {
        perror("initShm: ftruncate failed");
        return nullptr;
    }

    void* ptr = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
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
            perror("openShm: shm_open failed\n");
        }
        return nullptr;
    }

    struct stat statbuf;
    if (fstat(g_fd, &statbuf) == -1) {
        perror("openShm: fstat failed");
        return nullptr;
    }
    size_t totalSize = statbuf.st_size;

    void* ptr = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("openShm: mmap failed");
        return nullptr;
    }
    
    ShmLayout* layoutPtr = static_cast<ShmLayout*>(ptr);
    assert(layoutPtr->header.signature == 0xB0B);

    return layoutPtr;
}