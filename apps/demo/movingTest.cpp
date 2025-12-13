#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <iostream>
#include <filesystem>

int main() {
    Scene scene = Scene();
    SphereGeometry sphereG = {.pos = {0, 0, 5}, .radius=1};
    auto sphere = scene.createObject(sphereG, RED);
    scene.render(true);
    scene.setObjectTranslation(sphere, {1, 0, 0});
    scene.render(true);
    scene.setObjectTranslation(sphere, {0, 0.5, 0});
    scene.render(true);
}