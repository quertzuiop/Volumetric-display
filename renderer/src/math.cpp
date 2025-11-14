#pragma once
#include"types.h"

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
