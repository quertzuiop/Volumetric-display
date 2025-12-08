#include<array>
#include<vector>
#include<assert.h>
#include<cmath>
#include<cstdio>
#include "../include/types.h"

using namespace std;
//uses the algorithm for generation bayer matrices from wikipedia generalized to 3d
template <class T>
using Mat3d = vector<vector<vector<T>>>;

template <class T>
Mat3d<T> operator+(const Mat3d<T>& m, T a) { // add vector (d, d, d)
    for (const vector<vector<T>>&slice : m) {
        for (const vector<T>& row: slice) {
            for(T& element : row) {
                element += a;
            }
        }
    }
}
template <class T>
Mat3d<T> operator+(const Mat3d<T>& a, const Mat3d<T>& b) { // add vector (d, d, d)
    Mat3d<T> res;
    size_t Ax = a.size(), Ay = a[0].size(), Az = a[0][0].size();
    assert(Ax == b.size());
    assert(Ay == b[0].size());
    assert(Az == b[0][0].size());
    res.assign(Ax, vector<vector<T>>(Ay,vector<T>(Az)));

    for (int x = 0; x < Ax; ++x) {
        for (int y = 0; y < Ay; ++y) {
            for (int z = 0; z < Az; ++z) {
                res[x][y][z] = a[x][y][z] + b[x][y][z];
            }
        }
    }
    return res;
}
template <class T>
Mat3d<float> operator*(const Mat3d<T>& m, float factor) { // add vector (d, d, d)
    Mat3d<float> res;
    size_t Ax = m.size(), Ay = m[0].size(), Az = m[0][0].size();
    res.assign(Ax, vector<vector<float>>(Ay,vector<float>(Az)));

    for (int x = 0; x < Ax; ++x) {
        for (int y = 0; y < Ay; ++y) {
            for(int z = 0; z < Az; ++z) {
                res[x][y][z] = (float) m[x][y][z] * factor;
            }
        }
    }
}
Mat3d<int> operator*(const Mat3d<int>& m, int factor) { // add vector (d, d, d)
    Mat3d<int> res;
    size_t Ax = m.size(), Ay = m[0].size(), Az = m[0][0].size();
    res.assign(Ax, vector<vector<int>>(Ay,vector<int>(Az)));

    for (int x = 0; x < Ax; ++x) {
        for (int y = 0; y < Ay; ++y) {
            for(int z = 0; z < Az; ++z) {
                res[x][y][z] = m[x][y][z] * factor;
            }
        }
    }
    return res;
}
const int N=3;
const int bayerSize = pow(2, N);
const Mat3d<int> bayer2x2 = {{
    {
        {0, 2},
        {5, 7}
    }, {
        {6, 4},
        {3, 1}
    }
}};
const Mat3d<int> ones2 = {{
    {
        {1, 1},
        {1, 1}
    }, {
        {1, 1},
        {1, 1}
    }
}};

Mat3d<int> generate_ones(int size) {
    return Mat3d<int> (size, vector<vector<int>>(size, vector<int>(size, 1)));
}

Mat3d<int> kronecker3d(Mat3d<int> a, Mat3d<int> b) {
    Mat3d<int> res;
    size_t Ax = a.size(), Ay = a[0].size(), Az = a[0][0].size();
    size_t Bx = b.size(), By = b[0].size(), Bz = b[0][0].size();
    // printf("%d %d,%d %d, %d %d\n", Ax,Bx, Ay,By, Az,Bz);
    res.assign(Ax * Bx, vector<vector<int>>(Ay * By,vector<int>(Az * Bz)));
    for (int xa = 0; xa < Ax; ++xa) {
        for (int ya = 0; ya < Ay; ++ya) {
            for (int za = 0; za < Az; ++za) {
                for (int xb = 0; xb < Bx; ++xb) {
                    for (int yb = 0; yb < By; ++yb) {
                        for (int zb = 0; zb < Bz; ++zb) {
                            res[xb + xa*Bx][yb + ya*By][zb+za*Bz] = a[xa][ya][za] * b[xb][yb][zb];
                        }
                    }
                }
            }    
        }
    }
    return res;
}

Mat3d<float> generateBayer(int n) {
    Mat3d<int> bayer = bayer2x2;
    for (int i = 2; i <= n; ++i) {
        bayer = kronecker3d(ones2, bayer * 8) + kronecker3d(bayer2x2, generate_ones(pow(2, i-1)));
    }
    for (int x = 0; x < bayer.size(); ++x){
        for (int y = 0; y < bayer[0].size(); ++y){
            for (int z = 0; z < bayer[0][0].size(); ++z){
                printf("%d, ", bayer[x][y][z]);
            }
            printf("\n");
        }
        printf("------\n");
    }
    Mat3d<float> normalized;
    normalized.assign(bayer.size(), vector<vector<float>>(bayer[0].size(),vector<float>(bayer[0][0].size())));
    for (int x = 0; x < bayer.size(); ++x){
        for (int y = 0; y < bayer[0].size(); ++y){
            for (int z = 0; z < bayer[0][0].size(); ++z){
                normalized[x][y][z] = (bayer[x][y][z] + 0.5)/ (float) pow(bayerSize, 3);
            }       
        }
    }
    return normalized;
}
Mat3d<float> bayer = generateBayer(N);

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
    int ix = floor(pos.x)+35; //avoid negative index modulo error
    int iy = floor(pos.y)+35; //avoid negative index modulo error
    int iz = floor(pos.z)+1;
    bool r = bayer[(ix) % bayerSize][iy % bayerSize][iz % bayerSize] <= color.r;
    bool g = bayer[(ix+1) % bayerSize][(iy+2) % bayerSize][iz % bayerSize] <= color.g;
    bool b = bayer[(ix+1) % bayerSize][(iy+1) % bayerSize][(iz+1) % bayerSize] <= color.b;
    // printf("requested: %f %f %f, got: %d %d %d, compared with: %f %f %f\n", color.r, color.g, color.b, r, g, b, bayer[(ix+1) % bayerSize][iy % bayerSize][iz % bayerSize], bayer[ix% bayerSize][(iy+1) % bayerSize][iz % bayerSize], bayer[ix % bayerSize][iy % bayerSize][(iz+1) % bayerSize]);
    return {r, g, b};
}