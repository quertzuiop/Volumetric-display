#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include "renderer.h"
#include "types.h"

using namespace std;

const float height = 64;
const float maxSizeFactorXY = 1./sqrt(2); //45.25
const int cellCountZ = 8;
const int cellCountXY = floor(cellCountZ * maxSizeFactorXY);
const float cellSize = height/cellCountZ;

const float snakeMaxRadius = cellSize * 0.4;
const float snakeMinRadius = snakeMaxRadius/2.0;
const Color snakeColor1 = GREEN;
const Color snakeColor2 = BLUE;


const float appleRadius = cellSize * 0.4;
const Color appleColor = RED;

const int sleepTimeMs = 500;


unordered_map<char, Vec3<int>> inputMoveMap = {
    {'d', {-1, 0, 0}},
    {'a', {1, 0, 0}},
    {'w', {0, -1, 0}},
    {'s', {0, 1, 0}},
    {'j', {0, 0, -1}},
    {'i', {0, 0, 1}}
};

struct SnakeSegment {
    Vec3<int> p1;
    Vec3<int> p2;
    ObjectId objectId;
};

using Snake = vector<SnakeSegment>;

Vec3<float> getPosOfCell(Vec3<int> pos) {;
    // if (pos.x >= cellCountXY or pos.y >= cellCountXY or pos.z >= cellCountZ) {
    //     printf("(%d, %d, %d)\n", pos.x, pos.y, pos.z);
    //     throw invalid_argument("out of bounds");
    // }
    return {
        (pos.x+0.5) * cellSize - (cellCountXY * cellSize / 2.),
        (pos.y+0.5) * cellSize - (cellCountXY * cellSize / 2.),
        (pos.z+0.5) * cellSize
    };
}
Snake buildSnake(const vector<Vec3<int>>& snakePositions, Scene& scene) {
    Snake res;
    
    for (int i = 0; i < snakePositions.size() - 1; i++) {         
        CapsuleGeometry g = {.start = getPosOfCell(snakePositions[i]), .end = getPosOfCell(snakePositions[i+1])};
        
        Color currentColor = (i % 2 == 1) ? snakeColor2 : snakeColor1;
        
        auto segmentId = scene.createObject(g, currentColor);
        res.push_back((SnakeSegment){.p1 = snakePositions[i], .p2 = snakePositions[i+1], .objectId = segmentId});
    }
    return res;
}

Vec3<int> getNewApplePos(Snake snake) {
    Vec3<int> res;
    while (true) {
        res = {
            static_cast<int> (random() % cellCountXY),
            static_cast<int> (random() % cellCountXY),
            static_cast<int> (random() % cellCountZ)
        };
        bool collision = false;
        for (auto segment : snake) {
            if (segment.p1 == res or segment.p2 == res) {
                collision = true;
            }
        }
        if (not collision) {return res;}
    }
}
;
TextGeometry gameOverGeom1 = {
    .text = "GAME",
    .pos = {2.75*8, 0, 32},
    .size = 8,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};
TextGeometry gameOverGeom2 = {
    .text = "OVER",
    .pos = {2.75*8, 0, 22},
    .size = 8,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};

TextGeometry numberGeom = {
    .text = "3",
    .pos = {5, 0, 30},
    .size = 10,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};

void gameOver(Scene scene) {
    scene.wipe();
    scene.createObject(gameOverGeom1, RED);
    scene.createObject(gameOverGeom2, RED);
    scene.render();
    printf("GAME OVER\n");
    this_thread::sleep_for(chrono::milliseconds(2000));
    scene.wipe();
}

int main() {
    Scene scene = Scene();
    srand(time(0));

    auto countdown = scene.createObject(numberGeom, YELLOW);
    scene.render();
    this_thread::sleep_for(chrono::milliseconds(1000));

    numberGeom.text = "2";
    scene.setObjectGeometry(countdown, numberGeom);
    scene.render();
    this_thread::sleep_for(chrono::milliseconds(1000));

    numberGeom.text = "1";
    scene.setObjectGeometry(countdown, numberGeom);
    scene.render();
    this_thread::sleep_for(chrono::milliseconds(1000));

    scene.removeObject(countdown);

    
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

    Vec3<int> movementDirection = inputMoveMap['i'];
    
    Vec3<int> applePos = getNewApplePos(snake);

    auto apple = scene.createObject((SphereGeometry) {
        .pos = getPosOfCell(applePos),
        .radius = appleRadius
    }, appleColor);

    // scene.render();
    bool running = true;
    while (running) {
        auto keys = scene.getPressedKeys();

        for (int keyIndex = keys.size()-1; keyIndex >= 0; keyIndex--) {
            printf("keyIndex %d: %c\n", keyIndex, keys[keyIndex]);
            if (inputMoveMap.find(keys[keyIndex]) != inputMoveMap.end()) {
                auto newDirection = inputMoveMap[keys[keyIndex]];

                if (not (newDirection == (movementDirection * -1.0f))) { //avoid going back into itself
                    movementDirection = newDirection;
                    break;
                }
            }
        }
        
        headPos = snake[0].p1 + movementDirection;
        
        if (headPos == applePos) {
            auto snakeEnd = snake[snake.size()-1].p2;
            CapsuleGeometry g = {.start = getPosOfCell(snakeEnd), .end = getPosOfCell(snakeEnd), .radius = 0.0f};
            
            Color currentColor = (snake.size() % 2 == 1) ? snakeColor2 : snakeColor1;
            
            auto newSegment = scene.createObject(g, currentColor);

            snake.push_back((SnakeSegment) {
                .p1 = snake[snake.size()-1].p2,
                .p2 = snake[snake.size()-1].p2,
                .objectId = newSegment
            });
        }
        if (
            headPos.x < 0 || headPos.y < 0 || headPos.z < 0 ||
            headPos.x >= cellCountXY || headPos.y >= cellCountXY || headPos.z >= cellCountZ
        ) {
            gameOver(scene);
            running = false;
        }

        for (int j = snake.size() - 1; j >= 0; j--) {
            
            float t = (float)(snake.size() - 1 - j) / (float)(snake.size() - 1);
            float radius = snakeMinRadius + (snakeMaxRadius - snakeMinRadius) * t;

            Vec3<int> newSegmentStart;

            if (j == 0) { //head movement
                newSegmentStart = snake[0].p1 + movementDirection;
            } else {
                newSegmentStart = snake[j-1].p1;
            }
            
            snake[j].p2 = snake[j].p1;
            snake[j].p1 = newSegmentStart;

            scene.setObjectGeometry(snake[j].objectId, (CapsuleGeometry){
                .start = getPosOfCell(snake[j].p1), 
                .end = getPosOfCell(snake[j].p2),
                .radius = radius
            });
        }

        cout<<"new player pos" <<headPos << endl;
        for (auto& segment : snake) {
            if (segment.p2 == headPos) { //segments overlap, except for the very first p1 (the head)
                gameOver(scene);
                running = false;
            }
        }


        if (headPos == applePos) {
            auto newPos = getNewApplePos(snake);
            scene.setObjectGeometry(apple, (SphereGeometry) {
                .pos = getPosOfCell(newPos),
                .radius = appleRadius
            });
            applePos = newPos;
        }

        scene.render();

        this_thread::sleep_for(chrono::milliseconds(sleepTimeMs));
    }
}