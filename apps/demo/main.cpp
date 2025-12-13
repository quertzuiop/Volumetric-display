#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include <filesystem>

int main() {
    Scene scene = Scene();

    //Mesh mesh = loadMeshObj("assets/enjoybigfixed.obj");

    //SphereGeometry origin = {.pos = {0, 0, 20}, .radius = 5, .thickness= 0.5};
    float size = 45. / 10.;
    // CuboidGeometry cube = {.v1 = {-size/2, -size/2, -size/2}, .v2 = {size/2, size/2, 0} };
    CuboidGeometry cube = {.v1 = {-size/2, -size/2, -size/2}, .v2 = {size/2, size, size/2} };

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {

                ObjectId newObj = scene.createObject(cube, {(float) i /9, (float) k / 9, (float) j /9});
                // ObjectId newObj = scene.createObject(cube, {(float) i /9, (float) i /9, (float) i /9});
                scene.setObjectTranslation(newObj, {(45/2) - (size)*(i+1), (45/2) - (size)*(j+1), (size)*(k+1)});
            }
        }
    }

    // for (int i = 0; i < 10; ++i) {
    //     ObjectId newObj = scene.createObject(cube, {(float) i /9, 1 - ((float) i /9), 1});
    //     scene.setObjectTranslation(newObj, {(50/2) - (size)*(i), 0, size});
    // }

    // scene.createObject(origin, {0.5, 1, 0.5});
    //MeshGeometry geom = {.mesh = mesh, .isWireframe=true, .thickness=0.5};

    //auto meshObj = scene.createObject(geom, RED);
    //scene.setObjectRotation(meshObj, {3.1415/2, 0, 0}, {0, 0, 0});
    //scene.setObjectScale(meshObj, {20, 20, 20});
    scene.render(true);
}   