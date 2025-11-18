#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"

int main() {
    Scene scene = Scene();
    CuboidGeometry cubeGeom = {{-10, -10, 0}, {10, 10, 20}};
    Color red = {true, false, false};
    scene.createObject(cubeGeom, red);
    scene.render();
}