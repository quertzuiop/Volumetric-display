#include "renderer.h"
#include "types.h"

const float height = 64;
const float maxSizeFactorXY = 1./sqrt(2); //45.25
const int cellCountZ = 8;
const int cellCountXY = floor(cellCountZ * maxSizeFactorXY);
const float cellSize = height/cellCountZ;

Vec3 getPosOfCell(int x, int y, int z) {
    if (x >= cellCountXY or y >= cellCountXY or z >= cellCountZ) {
        printf("(%d, %d, %d)\n", x, y, z);
        throw invalid_argument("out of bounds");
    }
    return {
        x * cellSize - (cellCountXY * cellSize / 2.),
        y * cellSize - (cellCountXY * cellSize / 2.),
        z * cellSize - (cellCountZ * cellSize / 2.),
    };
}

int main() {
    Scene scene = Scene();

    auto boundaryCorner1 = getPosOfCell(0, 0, 0);
    boundaryCorner1.x -= cellSize/2;
    boundaryCorner1.y -= cellSize/2;
    boundaryCorner1.z -= cellSize/2;

    auto boundaryCorner2 = getPosOfCell(cellCountXY-1, cellCountXY-1, cellCountZ-1);
    boundaryCorner2.x += cellSize/2;
    boundaryCorner2.y += cellSize/2;
    boundaryCorner2.z += cellSize/2;

    CuboidGeometry boundaryGeometry = {
        .v1 = getPosOfCell(0, 0, 0),
        .v2 = getPosOfCell(cellCountXY-1, cellCountXY-1, cellCountZ-1),
        .thickness = 0.5,
        .isWireframe = true
    };

    scene.createObject(boundaryGeometry, WHITE);
    
    scene.render(true);
}