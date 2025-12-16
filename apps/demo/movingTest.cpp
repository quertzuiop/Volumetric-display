#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <iostream>
#include <filesystem>
#include <unistd.h>

int main() {
    Scene scene = Scene();
    SphereGeometry sphereG = {.pos = {0, 10, 20}, .radius=10};
    auto sphere = scene.createObject(sphereG, RED);
    printf("created object");
    //scene.render();
    //scene.setObjectTranslation(sphere, {1, 0, 0});
    //scene.render();
    //scene.setObjectTranslation(sphere, {0, 0.5, 0});
    while(true) {
        usleep(100000);
        scene.render();
    }
}