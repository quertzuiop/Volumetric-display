#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>
#include<variant>
#include<array>
#include<stdint.h>

struct Vec3 {
    float x;
    float y;
    float z;
};

using Mat4 = std::array<std::array<float, 4>, 4>;

struct Color1b {
    bool r;
    bool g;
    bool b;

    inline operator uint8_t() const {
        return
            static_cast<uint8_t>(
              (r ? (1u<<2) : 0u)
            | (g ? (1u<<1) : 0u)
            | (b ? 1u : 0u)
        );
    }
};
struct Color {
    float r;
    float g;
    float b;

    inline operator Color1b() const {
        return {
            r>0.5,
            g>0.5,
            b>0.5
        };
    }
};
const Color RED =     {1, 0, 0}; 
const Color GREEN =   {0, 1, 0};
const Color BLUE =    {0, 0, 1};
const Color WHITE =   {1, 1, 1};
const Color BLACK =   {0, 0, 0};
const Color CYAN =    {0, 1, 1};
const Color MAGENTA = {1, 0, 1};
const Color YELLOW =  {1, 1, 0};

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

using ObjectId = uint32_t; //::MAX is reserved for negative points

struct PointDisplayParams {
    uint16_t sliceIndex;
    uint16_t colIndex;
    uint16_t rowIndex;
    bool isDisplay1;
    bool isSide1;
};

struct RenderedPoint {
    ObjectId objectId;
    PointDisplayParams pointDisplayParams;
    Vec3 pos;
    Vec3 normal;
    Color1b color;
    ClippingBehavior clippingBehavior;
};

struct UpdatePatternPoint {
    PointDisplayParams pointDisplayParams;
    Vec3 pos;
    Vec3 normal;
    float ditherRank;
};

using Render = std::vector<RenderedPoint>;
using UpdatePattern = std::vector<UpdatePatternPoint>;


struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<std::pair<int, int>> edges;
    std::vector<std::array<int, 3>> faces;
};

