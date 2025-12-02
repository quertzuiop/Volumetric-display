#include<array>
#include<vector>
#include "types.h"

using namespace std;
//uses the algorithm for generation bayer matrices from wikipedia generalized to 3d
using Mat3d = vector<vector<vector<int>>>;
array<array<array<float, 2>, 2>, 2> bayer2x2 = {{
    {{
        {0, 2},
        {5, 7}
    }}, {{
        {6, 4},
        {3, 1}
    }}
}};

Mat3d kronecker3d(Mat3d a, Mat3d b) {
    Mat3d res;
    size_t Ax = a.size(), Ay = a[0].size(), Az = a[0][0].size();
    size_t Bx = b.size(), By = b[0].size(), Bz = b[0][0].size();

    // allocate result: dimensions = (Ax*Bx) x (Ay*By) x (Az*Bz)
    res.assign(Ax * Bx, vector<vector<int>>(Ay * By,vector<int>(Az * Bz)));
    for (int xa = 0; xa < Ax; ++xa) {
        for (int ya = 0; ya< Ay; ++ya) {
            for (int za = 0; za< Az; ++za) {
                for (int xb = 0; xb < Bx; ++xb) {
                    for (int yb = 0; yb< By; ++yb) {
                        for (int zb = 0; zb< Bz; ++zb) {
                            res[xb + xa*Bx][yb + ya*By][zb+za*Bz] = a[xa][ya][za] * b[xb][yb][zb];
                        }                
                    }
                }
            }                
        }
    }
}

Color1b dither(Color color, float ditherRank) {
    printf("rank: %f color: %f\n", ditherRank, color.r);
    bool r = powf(color.r, 1.5) > ditherRank;
    //bool g = powf(color.g, 1.5) > fmod(ditherRank + 0.333 ,1.);
    // bool r = false;
    bool g = false;
    bool b = powf(color.b, 1.5) > fmod(ditherRank + 0.666, 1.);
    return {r, g, b};
}
Color1b dither(Color color, Vec3 pos) {
    int ix = floor(pos.x)+35;
    int iy = floor(pos.y)+35;
    int iz = floor(pos.z);
    bool r =bayer2x2[ix%2][iy%2][iz%2] <= color.r;
    bool g =bayer2x2[(ix+1)%2][iy%2][iz%2] <= color.g;
    bool b =bayer2x2[ix%2][iy%2][(iz+1)%2] <= color.b;
    printf("requested: %f %f %f, got: %d %d %d\n", color.r, color.g, color.b, r, g, b);
    return {r, g, b};
}