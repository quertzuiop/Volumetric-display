#include"types.h"
#include"linalg.h"
#include <algorithm>
#include <iostream>

Mat4 matMul(const Mat4& mat1, const Mat4& mat2) {
    auto a = sgn(-2);
    Mat4 res;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                res[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    return res;
}

void Mesh::center(float padding) {
    Vec3<float> min = {__FLT_MAX__, __FLT_MAX__, __FLT_MAX__};
    Vec3<float> max = {-__FLT_MAX__, -__FLT_MAX__, -__FLT_MAX__};

    Vec3<float> boundsMin = {-32 + padding, -32 + padding, 0 + padding};
    Vec3<float> boundsMax = {32 - padding, 32 - padding, 64 - padding};

    auto boundsCenter = (boundsMin + boundsMax) * 0.5;

    for (const auto& v : vertices) {
        min.x = min.x < v.x ? min.x : v.x;
        min.y = min.y < v.y ? min.y : v.y;
        min.z = min.z < v.z ? min.z : v.z;
    
        max.x = max.x > v.x ? max.x : v.x;
        max.y = max.y > v.y ? max.y : v.y;
        max.z = max.z > v.z ? max.z : v.z;
    }

    auto meshCenter = (min + max) * 0.5;
    
    auto meshSize = max - min;
    auto targetSize = boundsMax - boundsMin;

    float scaleX = (meshSize.x > 0) ? targetSize.x / meshSize.x : 1.0f;
    float scaleY = (meshSize.y > 0) ? targetSize.y / meshSize.y : 1.0f;
    float scaleZ = (meshSize.z > 0) ? targetSize.z / meshSize.z : 1.0f;

    float scaleFactor = std::min({scaleX, scaleY, scaleZ});
    std::cout<<"calculated mesh center: "<<meshCenter << ", bounds center: " <<boundsCenter << ", scale factor: " <<scaleFactor<<std::endl;
    std::cout << "min: " << min << ", max: " << max<<std::endl;
    for (auto& v : vertices) {
        v = v - meshCenter;
        v = v * scaleFactor;
        v = v + boundsCenter;
    }
}