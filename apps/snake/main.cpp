#include <iostream>
#include "renderer.h"
#include "types.h"

using namespace std;

const float snakeMaxRadius = 3.;
const float snakeMinRadius = 1.5;
const Color snakeColor = GREEN;

const float height = 64;
const float maxSizeFactorXY = 1./sqrt(2); //45.25
const int cellCountZ = 8;
const int cellCountXY = floor(cellCountZ * maxSizeFactorXY);
const float cellSize = height/cellCountZ;

unordered_map<string, Vec3<int>> inputMoveMap = {
    {"a", {-1, 0, 0}},
    {"d", {1, 0, 0}},
    {"s", {0, -1, 0}},
    {"w", {0, 1, 0}},
    {"j", {0, 0, -1}},
    {"i", {0, 0, 1}}
};

struct SnakeSegment {
    Vec3<int> p1;
    Vec3<int> p2;
    float radius;
    ObjectId objectId;
};

Vec3<float> getPosOfCell(Vec3<int> pos) {;
    if (pos.x >= cellCountXY or pos.y >= cellCountXY or pos.z >= cellCountZ) {
        printf("(%d, %d, %d)\n", pos.x, pos.y, pos.z);
        throw invalid_argument("out of bounds");
    }
    return {
        (pos.x+0.5) * cellSize - (cellCountXY * cellSize / 2.),
        (pos.y+0.5) * cellSize - (cellCountXY * cellSize / 2.),
        (pos.z+0.5) * cellSize
    };
}

vector<SnakeSegment> buildSnake(const vector<Vec3<int>>& snakePositions, Scene& scene) {
    vector<SnakeSegment> res;

    for (int i = 0; i < snakePositions.size() - 1; i++) {
        float t = (float)(snakePositions.size() - i-2) / (float)(snakePositions.size()-2);
        float radius = snakeMinRadius + (snakeMaxRadius - snakeMinRadius) * t;

        CapsuleGeometry g = {.start = getPosOfCell(snakePositions[i]), .end = getPosOfCell(snakePositions[i+1]), .radius = radius};
        auto segment = scene.createObject(g, snakeColor);
        res.push_back((SnakeSegment){.p1 = snakePositions[i], .p2 = snakePositions[i+1], .radius = radius, .objectId = segment});
    }
    return res;
}

int main() {
    Scene scene = Scene();

    auto boundaryCorner1 = getPosOfCell({0, 0, 0});
    boundaryCorner1 = boundaryCorner1 - cellSize/2-0.5;

    auto boundaryCorner2 = getPosOfCell({cellCountXY-1, cellCountXY-1, cellCountZ-1});
    boundaryCorner2 = boundaryCorner2 + cellSize/2-0.5;


    CuboidGeometry boundaryGeometry = {
        .v1 = boundaryCorner1,
        .v2 = boundaryCorner2,
        .thickness = 0.7,
        .isWireframe = true
    };

    scene.createObject(boundaryGeometry, WHITE);

    Vec3<int> headPos = {cellCountXY/2, cellCountXY/2, cellCountZ/2};
    auto headRenderPos = getPosOfCell(headPos);

    SphereGeometry playerGeometry = {
        .pos = headRenderPos,
        .radius = 3
    };
    auto headSegment = scene.createObject(
        (CapsuleGeometry){getPosOfCell(headPos), getPosOfCell(headPos + (Vec3<int>){0, 0, -1})},
        GREEN
    );

    //LOGIC
    auto snake = buildSnake({
        headPos,
        headPos + (Vec3<int>){0, 0, -1},
        headPos + (Vec3<int>){0, 1, -1},
        headPos + (Vec3<int>){1, 1, -1}
    }, scene);

    for (int x = 0; x < cellCountXY; x++) {
        for (int y = 0; y < cellCountXY; y++) {
            for (int z = 0; z < cellCountZ; z++) {
                SphereGeometry G = {.pos = getPosOfCell({x, y, z}), .radius = 3};
                //scene.createObject(G, GREEN);
            }
        }
    }
    for (int i = 0; i < 10; i++) {
        scene.render(true);
        
        string input;
        cin >> input;
        auto movement = inputMoveMap[input];
        
        for (int i = snake.size() - 1; i >= 0; i--) { //not including last segment
            Vec3<int> newSegmentStart;

            if (i == 0) { //head movement
                newSegmentStart = snake[i].p1 + movement;
            } else {
                newSegmentStart = snake[i-1].p1;
            }
            
            snake[i].p2 = snake[i].p1;
            snake[i].p1 = newSegmentStart;

            cout<<"p1: " << snake[i].p1 << endl;
            cout<<"p2: " << snake[i].p2 << endl;

            scene.setObjectGeometry(snake[i].objectId, (CapsuleGeometry){
                .start = getPosOfCell(snake[i].p1), 
                .end = getPosOfCell(snake[i].p2),
                .radius = snake[i].radius
            });
        }

        headPos = snake[i].p1;
        cout<<"new player pos" <<headPos << endl;

        if (
            headPos.x < 0 || headPos.y < 0 || headPos.z < 0 ||
            headPos.x > cellCountXY || headPos.y > cellCountXY || headPos.z > cellCountZ
        ) {
            printf("GAME OVER\n");
            break;
        }

        // scene.setObjectGeometry(player, (SphereGeometry){.pos=getPosOfCell(headPos), .radius=3});
    }
}