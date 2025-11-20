#pragma once
#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>
#include<variant>

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

struct Transformation {
    Vec3 translation = { 0, 0, 0 };
    Vec3 rotation = { 0, 0, 0 };
    Vec3 scale = { 1, 1, 1 };
};
enum ClippingBehavior {
    ADD,
    OVERWRITE,
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

using ObjectId = uint32_t;

struct PointDisplayParams {
    uint16_t frameIndex;
    uint16_t rowIndex;
    uint16_t colIndex;
    bool isDisplay1;
};

struct RenderedPoint {
    ObjectId objectId;
    PointDisplayParams pointDisplayParams;
    Vec3 pos;
    Vec3 normal;
    Color color;
    ClippingBehavior clippingBehavior;
};

struct UpdatePatternPoint {
    PointDisplayParams pointDisplayParams;
    Vec3 pos;
    Vec3 normal;
};

using Render = std::vector<RenderedPoint>;
using UpdatePattern = std::vector<UpdatePatternPoint>;


struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<std::pair<int, int>> edges;
    std::vector<std::array<int, 3>> faces;
};

