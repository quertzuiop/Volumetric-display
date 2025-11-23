#pragma once
#include"../include/types.h"

std::tuple<Vec3, Vec3> arrangeBoundingBox(const Vec3& p1, const Vec3& p2) {
    return { {
        std::min(p1.x, p2.x),
        std::min(p1.y, p2.y),
        std::min(p1.z, p2.z),
    }, {
        std::max(p1.x, p2.x),
        std::max(p1.y, p2.y),
        std::max(p1.z, p2.z),
    } };
}

Mat4 matMul(const Mat4& mat1, const Mat4& mat2) {
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