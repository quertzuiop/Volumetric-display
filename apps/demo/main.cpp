#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"

int main() {
    Scene scene = Scene();
    CuboidGeometry cubeGeom = {{-10, -10, 0}, {10, 10, 20}};
    SphereGeometry sphereGeom = {{10, 20, 40}, 5};
    //ParticleGeometry capsule = {{-10, -10}}
    Color red = {true, false, false};
    Color cyan = {false, true, true};
    scene.createObject(cubeGeom, red );
    scene.createObject(sphereGeom, cyan);
    scene.render();
}