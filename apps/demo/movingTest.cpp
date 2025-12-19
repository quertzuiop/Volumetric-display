#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <chrono>

int main() {
    using namespace std::chrono;

    Scene scene = Scene();
    SphereGeometry G = {.pos = {0, 0, 32}, .radius=6};
    auto sphere = scene.createObject(G, RED);
    printf("created object");
    //scene.render();
    //scene.setObjectTranslation(sphere, {1, 0, 0});
    //scene.render();
    //scene.setObjectTranslation(sphere, {0, 0.5, 0});
    double i = 0;
    while(true) {
        auto start = steady_clock::now();
        usleep(10000);
        printf("X: %f\n", static_cast<float>(sin(i/10.) * 10.));
        scene.setObjectTranslation(sphere, {static_cast<float>(sin(i/100.) * 10.), 0, 0});
        scene.render();
        i++;
        auto fps = 1/duration<double>(steady_clock::now() - start).count();
        printf("fps: %f\n", fps);
    }
}   