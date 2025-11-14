#pragma once
#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Color {
    bool r;
    bool g;
    bool b;
};

struct PointDisplayParams {
    uint8_t frameIndex;
    uint8_t rowIndex;
    bool isDisplay1;
};

//struct Point {
//    Vec3 pos;
//    Vec3 normal;
//    Color color;
//    PointDisplayParams pointDisplayParams;
//    Vec3 shmAddr;
//};

struct GridParams {
    Vec3 boundingBoxMin;
    Vec3 boundingBoxMax;
    int gridSize;
    Vec3 cellSizes;
};

using Point = std::pair<Vec3, Vec3>;
using ptCloud = std::vector<Point>;

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<std::pair<int, int>> edges;
    std::vector<std::array<int, 3>> faces;
};
