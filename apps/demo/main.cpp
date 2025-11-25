#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <filesystem>

int main() {
    Scene scene = Scene();
    Mesh mesh = loadMeshObj("assets/enjoybigfixed.obj");

    SphereGeometry origin = {.pos = {0, 0, 1}, .radius = 1};
    scene.createObject(origin, WHITE);
    MeshGeometry geom = {.mesh = mesh, .isWireframe=true, .thickness=0.5};

    auto meshObj = scene.createObject(geom, RED);
    //scene.setObjectRotation(meshObj, {3.1415/2, 0, 0}, {0, 0, 0});
    scene.setObjectScale(meshObj, {20, 20, 20}, {0, 0, 0});
    scene.render();
}