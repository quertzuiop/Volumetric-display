#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <iostream>
#include <filesystem>

int main() {
    std::cout<<"loading scene"<<endl;
    Scene scene = Scene();
    std::cout<<"loaded"<<endl;

    Mesh mesh = loadMeshObj("assets/test.obj");
    std::cout<<"mesh"<<endl;

    // scene.createObject(origin, {0.5, 1, 0.5});
    MeshGeometry geom = {.mesh = mesh, .isWireframe=true, .thickness=0.5};

    auto meshObj = scene.createObject(geom, WHITE);
    scene.setObjectTranslation(meshObj, {0, 0, 0});
    scene.setObjectScale(meshObj, {10, 10, 10});
    scene.render();
}   