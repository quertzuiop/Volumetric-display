#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <filesystem>

int main() {
    Scene scene = Scene();

    Mesh mesh = loadMeshObj("assets/test.obj");


    // scene.createObject(origin, {0.5, 1, 0.5});
    MeshGeometry geom = {.mesh = mesh, .isWireframe=true, .thickness=0.5};

    auto meshObj = scene.createObject(geom, RED);
    scene.setObjectTranslation(meshObj, {0, 0, 32});
    scene.setObjectScale(meshObj, {10, 30, 30});
    scene.render();
}   