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
    ObjectId objectId1; // tail half
    ObjectId objectId2; // head half
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
        Color currentColor = (i % 2 == 1) ? snakeColor2 : snakeColor1;
        
        Vec3<float> p1_pos = getPosOfCell(snakePositions[i]);
        Vec3<float> p2_pos = getPosOfCell(snakePositions[i+1]);

        CapsuleGeometry g1 = {.start = p2_pos, .end = p1_pos};
        CapsuleGeometry g2 = {.start = p1_pos, .end = p1_pos};
        
        auto id1 = scene.createObject(g1, currentColor);
        auto id2 = scene.createObject(g2, currentColor);
        
        res.push_back((SnakeSegment){
            .p1 = snakePositions[i], 
            .p2 = snakePositions[i+1], 
            .objectId1 = id1, 
            .objectId2 = id2
        });
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
    .pos = {2.75*8, 0, 44},
    .size = 8,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};
TextGeometry gameOverGeom2 = {
    .text = "OVER",
    .pos = {2.75*8, 0, 32},
    .size = 8,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};
TextGeometry scoreGeom = {
    .text = "",
    .pos = {2.75*8, 0, 20},
    .size = 4.5,
    .thickness = 0.6,
    .orientation = TextOrientation::POS_Y
};


TextGeometry numberGeom = {
    .text = "3",
    .pos = {5, 0, 30},
    .size = 10,
    .thickness = 0.8,
    .orientation = TextOrientation::POS_Y
};

void gameOver(Scene scene, int score) {
    scene.wipe();
    scene.createObject(gameOverGeom1, RED);
    scene.createObject(gameOverGeom2, RED);
    scoreGeom.text = format("SCR {}", score);
    scene.createObject(scoreGeom, CYAN);
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
    int appleCount = 0;
    while (running) {
        auto keys = scene.getPressedKeys();

        for (int keyIndex = keys.size()-1; keyIndex >= 0; keyIndex--) {
            if (inputMoveMap.find(keys[keyIndex]) != inputMoveMap.end()) {
                auto newDirection = inputMoveMap[keys[keyIndex]];
                if (not (newDirection == (movementDirection * -1.0f))) { 
                    movementDirection = newDirection;
                    break;
                }
            }
        }
        
        headPos = snake[0].p1 + movementDirection;
        
        // --- 1. Apple Logic & Growth ---
        bool ateApple = false;
        if (headPos == applePos) {
            ateApple = true;
            appleCount++;

            auto snakeEnd = snake.back().p2;
            CapsuleGeometry g = {.start = getPosOfCell(snakeEnd), .end = getPosOfCell(snakeEnd), .radius = 0.0f};
            
            Color currentColor = (snake.size() % 2 == 1) ? snakeColor2 : snakeColor1;
            
            auto newSegment1 = scene.createObject(g, currentColor);
            auto newSegment2 = scene.createObject(g, currentColor);

            snake.push_back((SnakeSegment) {
                .p1 = snakeEnd,
                .p2 = snakeEnd,
                .objectId1 = newSegment1,
                .objectId2 = newSegment2
            });
        } 

        if (
            headPos.x < 0 || headPos.y < 0 || headPos.z < 0 ||
            headPos.x >= cellCountXY || headPos.y >= cellCountXY || headPos.z >= cellCountZ
        ) {
            gameOver(scene, appleCount);
            running = false;
            continue;
        }

        // self Collision
        for (int j = 1; j < snake.size(); j++) {
            if (snake[j].p1 == headPos) { 
                gameOver(scene, appleCount);
                running = false;
            }
        }
        if (!running) continue;

        vector<Vec3<int>> next_p1(snake.size());
        next_p1[0] = headPos;
        for (int j = 1; j < snake.size(); j++) {
            next_p1[j] = snake[j-1].p1;
        }

        // animation
        int numFrames = 10; // Splitting the movement into 10 smooth steps
        int frameSleepMs = sleepTimeMs / numFrames;

        for (int frame = 1; frame <= numFrames; frame++) {
            float f = (float)frame / numFrames; // f goes from 0.0 to 1.0

            for (int j = snake.size() - 1; j >= 0; j--) {
                float t = (float)(snake.size() - 1 - j) / (float)(snake.size() - 1);
                float radius = snakeMinRadius + (snakeMaxRadius - snakeMinRadius) * t;

                Vec3<float> pivot = getPosOfCell(snake[j].p1);
                Vec3<float> p2_pos = getPosOfCell(snake[j].p2);
                Vec3<float> next_pos = getPosOfCell(next_p1[j]);

                // Interpolate without relying on Vec3 operator overloads
                Vec3<float> tailPos = {
                    p2_pos.x + (pivot.x - p2_pos.x) * f,
                    p2_pos.y + (pivot.y - p2_pos.y) * f,
                    p2_pos.z + (pivot.z - p2_pos.z) * f
                };

                Vec3<float> headPosVis = {
                    pivot.x + (next_pos.x - pivot.x) * f,
                    pivot.y + (next_pos.y - pivot.y) * f,
                    pivot.z + (next_pos.z - pivot.z) * f
                };

                // Apply geometry
                scene.setObjectGeometry(snake[j].objectId1, (CapsuleGeometry){
                    .start = tailPos, 
                    .end = pivot,
                    .radius = radius
                });
                scene.setObjectGeometry(snake[j].objectId2, (CapsuleGeometry){
                    .start = pivot, 
                    .end = headPosVis,
                    .radius = radius
                });
            }
            scene.render();
            this_thread::sleep_for(chrono::milliseconds(frameSleepMs));
        }

        cout << "new player pos" << headPos << endl;
        for (int j = snake.size() - 1; j >= 0; j--) {
            snake[j].p2 = snake[j].p1;
            snake[j].p1 = next_p1[j];
        }

        // --- 6. Respawn Apple ---
        if (ateApple) {
            auto newPos = getNewApplePos(snake);
            scene.setObjectGeometry(apple, (SphereGeometry) {
                .pos = getPosOfCell(newPos),
                .radius = appleRadius
            });
            applePos = newPos;
        }
    }
}