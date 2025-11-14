#include<array>
#include<vector>
#include "types.h"

using namespace std;

inline int packIndex(int ix, int iy, int iz);

inline array<int, 3> unpackIndex(int packed);

int calculateIndex(const GridParams& params, const Vec3& ptCoords);

vector<int> calculateIndicesFromBB(const GridParams& params, const Vec3& min, const Vec3& max, float padding = 0.);

tuple<unordered_map<int, ptCloud>, GridParams> buildGrid(const ptCloud& points, int ptsPerCell);
