#include "shm.h"
#include <chrono>
#include <wiringPi.h>
#include <iostream>
#include <zmq.hpp>

using namespace std;
const double targetFps = 24.;

void waitForFall() {
    while (digitalRead(1) == HIGH) {}
}

int main() {    
    volatile ShmLayout* shmPointer = openShm("vdshm");

    int64_t lastFrameStart;
    int frameNum = 0;

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP); // REP = Reply socket

    // Bind to all interfaces on port 5555
    socket.bind("tcp://*:5555");
    printf("Pi 4 Server started. Waiting for data...\n");

    while (true) {
        zmq::message_t request;

        // 1. Receive data (blocking)
        auto result = socket.recv(request, zmq::recv_flags::none);
        
        // Parse the 2 numbers (assuming float for simplicity)
        float* data = static_cast<float*>(request.data());
        std::cout << "Received: " << data[0] << ", " << data[1] << std::endl;

        // 2. Process data (Do your logic here)
        float response_val = data[0] + data[1]; // Example: return the sum

        // 3. Send reply back
        zmq::message_t reply(sizeof(float));
        memcpy(reply.data(), &response_val, sizeof(float));
        socket.send(reply, zmq::send_flags::none);
    }

    printf("speed regulator setup succesfully\n");
    while (true) {
        auto currentTime = chrono::time_point_cast<chrono::nanoseconds>(chrono::steady_clock::now()).time_since_epoch().count();

        if (frameNum >= 1) {
            auto frameDuration =  chrono::steady_clock::now().time_since_epoch().count() - lastFrameStart;
            shmPointer->nextFrameStart = currentTime;
            shmPointer->nextFrameDuration = frameDuration;
        }
        lastFrameStart = currentTime;
    }
    // auto start = chrono::steady_clock::now();
    // auto a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    // for (int i = 0; i < 48000; i++) {
    //     a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    // }
    // auto end = chrono::steady_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    // printf("Total time for 10000 calls: %lld microseconds\n", duration);
}