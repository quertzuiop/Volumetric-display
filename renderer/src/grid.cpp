#include<array>
#include<iostream>
#include "../include/types.h"

using namespace std;
inline int packIndex(int ix, int iy, int iz) {
    return (ix << 22) | (iy << 11) | iz;
}

inline array<int, 3> unpackIndex(int packed) {
    return { packed >> 22, (packed >> 11) & ((1 << 11) - 1), packed & ((1 << 11) - 1) };
}
int calculateIndex(const GridParams& params, const Vec3& ptCoords) {
    const Vec3& min = params.boundingBoxMin;
    const Vec3& cellSizes = params.cellSizes;

    int bitMax = pow(2, 10);

    int ix = floor((ptCoords.x - min.x) / cellSizes.x);
    int iy = floor((ptCoords.y - min.y) / cellSizes.y);
    int iz = floor((ptCoords.z - min.z) / cellSizes.z);

    //ix - (10 bits), iy - (11 bits), iz - (11bits)
    return packIndex(ix, iy, iz);

}
vector<int> calculateIndicesFromBB(const GridParams& params, const Vec3& min, const Vec3& max, float padding = 0.) {
    const Vec3& boxMin = params.boundingBoxMin;
    const Vec3& boxMax = params.boundingBoxMax;

    const Vec3& cellSizes = params.cellSizes;

    int minIx = floor((min.x - boxMin.x - padding) / cellSizes.x);
    int minIy = floor((min.y - boxMin.y - padding) / cellSizes.y);
    int minIz = floor((min.z - boxMin.z - padding) / cellSizes.z);

    int maxIx = floor((max.x - boxMin.x + padding) / cellSizes.x);
    int maxIy = floor((max.y - boxMin.y + padding) / cellSizes.y);
    int maxIz = floor((max.z - boxMin.z + padding) / cellSizes.z);
    //printf("---indices: bounding box min and max: %f %f %f, %f %f %f\n", min.x, min.y, min.z, max.x, max.y, max.z);
    //printf("---min values: %d %d %d\n--max values: %d %d %d\n", minIx, minIy, minIz, maxIx, maxIy, maxIz);
    vector<int> res;
    res.reserve((maxIx - minIx + 1) * (maxIy - minIy + 1) * (maxIz - minIz + 1));
    for (int ix = minIx; ix <= maxIx; ix++) {
        for (int iy = minIy; iy <= maxIy; iy++) {
            for (int iz = minIz; iz <= maxIz; iz++) {
                res.push_back(packIndex(ix, iy, iz));
            }
        }
    }
    return res;
}
tuple<unordered_map<int, UpdatePattern>, GridParams> buildGrid(const UpdatePattern& points, int ptsPerCell) {
    int numPoints = points.size();
    int nCells = numPoints / ptsPerCell;
    int gridSize = ceil(pow(nCells, 1. / 3.));

    //find bounding box
    Vec3 Min = { 0, 0, 0 };
    Vec3 Max = { 0, 0, 0 };

    for (const UpdatePatternPoint& pt : points) {
        Vec3 ptCoords = pt.pos;
        Min.x = min(Min.x, ptCoords.x);
        Min.y = min(Min.y, ptCoords.y);
        Min.z = min(Min.z, ptCoords.z);

        Max.x = max(Max.x, ptCoords.x);
        Max.y = max(Max.y, ptCoords.y);
        Max.z = max(Max.z, ptCoords.z);
    }

    cout << "grid size: " << gridSize << endl;
    printf("Min: %f %f %f", Min.x, Min.y, Min.z);
    printf("Max: %f %f %f", Max.x, Max.y, Max.z);

    float cellSizeZ = (Max.z - Min.z) / gridSize;
    float cellSizeX = (Max.x - Min.x) / gridSize;
    float cellSizeY = (Max.y - Min.y) / gridSize;

    GridParams params = {
        Min,
        Max,
        gridSize,
        Vec3 {cellSizeX, cellSizeY, cellSizeZ}
    };
    cout << "cells sizes: " << cellSizeX << ", " << cellSizeY << ", " << cellSizeZ << endl;
    unordered_map<int, UpdatePattern> map;

    for (int i = 0; i < points.size(); i++) {
        int packed = calculateIndex(params, points[i].pos);
        auto& mapVal = map[packed];
        mapVal.push_back(std::move(points[i]));
        //array<int, 3> unpacked = unpackIndex(packed);
        //if (unpacked != array<int, 3>{ix, iy, iz}) cout << "failed. reconstructed:" << endl;
        //cout << bitset<32>(packed).to_string() << endl;
    }
    return { map, params };
}
