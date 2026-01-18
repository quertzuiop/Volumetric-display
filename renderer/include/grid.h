#pragma once

#include<array>
#include<vector>
#include "types.h"

using namespace std;

inline int packIndex(int ix, int iy, int iz);

inline array<int, 3> unpackIndex(int packed);

int calculateIndex(const GridParams& params, const Vec3<float>& ptCoords);

vector<int> calculateIndicesFromBB(const GridParams& params, const Vec3<float>& min, const Vec3<float>& max, float padding = 0.);

tuple<unordered_map<int, UpdatePattern>, GridParams> buildGrid(const UpdatePattern& points, int ptsPerCell);
