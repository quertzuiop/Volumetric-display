#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <filesystem>

int main() {
    Scene scene = Scene();
    SphereGeometry geom = {.pos = {0, 0, 20}, .radius = 15};

    //Mesh mesh = loadMeshObj("assets/test.obj");
    //printf("n vert. of mesh: %d\n", mesh.vertices.size());
    //MeshGeometry geom = {mesh, true};
    scene.createObject(geom, MAGENTA);
    scene.createObject(geom, YELLOW);

    scene.render();
}